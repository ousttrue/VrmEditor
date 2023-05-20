#include "gl3renderer.h"
#include "app.h"
#include "image.h"
#include "rendering_env.h"
#include <DirectXMath.h>
#include <GL/glew.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <cuber/mesh.h>
#include <grapho/gl3/pbr.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/texture.h>
#include <grapho/gl3/vao.h>
#include <imgui.h>
#include <iostream>
#include <map>
#include <unordered_map>
#include <variant>
#include <vrm/fileutil.h>
#include <vrm/gltf.h>
#include <vrm/runtimescene/deformed_mesh.h>

static auto vertex_shader_text = u8R"(#version 400
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
in vec3 vPosition;
in vec3 vNormal;
in vec2 vUv;
out vec3 normal;
out vec2 uv;
void main()
{
  gl_Position = Projection * View * Model * vec4(vPosition, 1.0);
  normal = vNormal;
  uv = vUv;
}
)";

static auto fragment_shader_text = u8R"(#version 400
in vec3 normal;
in vec2 uv;
out vec4 FragColor;
uniform vec4 color=vec4(1, 1, 1, 1);
uniform float cutoff=1;
uniform sampler2D colorTexture;
void main()
{
  vec4 texel = color * texture(colorTexture, uv);
  if(texel.a < cutoff)
  {
    discard;
  }
  FragColor = texel;
};
)";

static auto shadow_vertex_text = u8R"(#version 400
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform mat4 Shadow;
in vec3 vPosition;
in vec3 vNormal;
in vec2 vUv;
out vec3 normal;
out vec2 uv;
void main()
{
  gl_Position = Projection * View * Shadow * Model * vec4(vPosition, 1.0);
  normal = vNormal;
  uv = vUv;
}
)";

static auto shadow_fragment_text = u8R"(#version 400
in vec3 normal;
in vec2 uv;
out vec4 FragColor;
uniform sampler2D colorTexture;
void main()
{
  FragColor = vec4(0, 0, 0, 0.7);
};
)";

namespace glr {

static std::expected<std::shared_ptr<Image>, std::string>
ParseImage(const gltfjson::format::Root& root,
           const gltfjson::format::Bin& bin,
           const gltfjson::format::Image& image)
{
  std::span<const uint8_t> bytes;
  if (auto bufferView = image.BufferView) {
    if (auto buffer_view = bin.GetBufferViewBytes(root, *bufferView)) {
      bytes = *buffer_view;
    } else {
      return std::unexpected{ buffer_view.error() };
    }
  } else if (image.Uri.size()) {
    if (auto buffer_view = bin.Dir->GetBuffer(image.Uri)) {
      bytes = *buffer_view;
    } else {
      return std::unexpected{ buffer_view.error() };
    }
  } else {
    return std::unexpected{ "not bufferView nor uri" };
  }
  auto name = image.Name;
  auto ptr = std::make_shared<Image>(name);
  if (!ptr->Load(bytes)) {
    return std::unexpected{ "Image: fail to load" };
  }
  return ptr;
}

class Gl3Renderer
{
  // https://stackoverflow.com/questions/12875652/how-can-i-use-a-stdmap-with-stdweak-ptr-as-key
  using MeshWeakPtr = std::weak_ptr<runtimescene::BaseMesh>;

  std::map<uint32_t, std::shared_ptr<Image>> m_imageMap;
  std::map<uint32_t, std::shared_ptr<grapho::gl3::Texture>> m_srgbTextureMap;
  std::map<uint32_t, std::shared_ptr<grapho::gl3::Texture>> m_linearTextureMap;
  std::map<uint32_t, std::shared_ptr<grapho::gl3::PbrMaterial>> m_materialMap;

  std::map<MeshWeakPtr,
           std::shared_ptr<grapho::gl3::Vao>,
           std::owner_less<MeshWeakPtr>>
    m_drawableMap;

  std::shared_ptr<grapho::gl3::Texture> m_white;
  std::shared_ptr<grapho::gl3::ShaderProgram> m_shadow;

  Gl3Renderer()
  {
    static uint8_t white[] = { 255, 255, 255, 255 };
    m_white = grapho::gl3::Texture::Create({
      1,
      1,
      grapho::PixelFormat::u8_RGBA,
      grapho::ColorSpace::sRGB,
      white,
    });

    // if (auto program = grapho::gl3::ShaderProgram::Create(
    //       vertex_shader_text, fragment_shader_text)) {
    //   m_program = *program;
    // } else {
    //   App::Instance().Log(LogLevel::Error) << program.error();
    // }
    if (auto shadow = grapho::gl3::ShaderProgram::Create(
          shadow_vertex_text, shadow_fragment_text)) {
      m_shadow = *shadow;
    } else {
      App::Instance().Log(LogLevel::Error) << shadow.error();
    }
  }

  ~Gl3Renderer() {}

public:
  static Gl3Renderer& Instance()
  {
    static Gl3Renderer s_instance;
    return s_instance;
  }

  void Release() { m_drawableMap.clear(); }

  std::shared_ptr<Image> GetOrCreateImage(const gltfjson::format::Root& root,
                                          const gltfjson::format::Bin& bin,
                                          std::optional<uint32_t> id)
  {
    if (!id) {
      return {};
    }

    auto found = m_imageMap.find(*id);
    if (found != m_imageMap.end()) {
      return found->second;
    }

    if (auto image = ParseImage(root, bin, root.Images[*id])) {
      m_imageMap.insert({ *id, *image });
      return *image;
    } else {
      App::Instance().Log(LogLevel::Error) << image.error();
      return {};
    }
  }

  std::shared_ptr<grapho::gl3::Texture> GetOrCreateTexture(
    const gltfjson::format::Root& root,
    const gltfjson::format::Bin& bin,
    std::optional<uint32_t> id,
    libvrm::gltf::ColorSpace colorspace)
  {
    if (!id) {
      return {};
    }

    auto& map = colorspace == libvrm::gltf::ColorSpace::sRGB
                  ? m_srgbTextureMap
                  : m_linearTextureMap;

    auto found = map.find(*id);
    if (found != map.end()) {
      return found->second;
    }

    auto& src = root.Textures[*id];
    if (!src.Source) {
      return {};
    }

    auto image = GetOrCreateImage(root, bin, src.Source);

    auto texture = grapho::gl3::Texture::Create({
      image->Width(),
      image->Height(),
      grapho::PixelFormat::u8_RGBA,
      colorspace == libvrm::gltf::ColorSpace::sRGB ? grapho::ColorSpace::sRGB
                                                   : grapho::ColorSpace::Linear,
      image->Pixels(),
    });

    if (src.Sampler) {
      auto& sampler = root.Samplers[*src.Sampler];
      texture->Bind();
      glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)sampler.MagFilter);
      glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)sampler.MinFilter);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int)sampler.WrapS);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int)sampler.WrapT);
      texture->Unbind();
    } else {
      // TODO: default sampler
    }

    map.insert(std::make_pair(*id, texture));
    return texture;
  }

  std::shared_ptr<grapho::gl3::PbrMaterial> GetOrCreateMaterial(
    const gltfjson::format::Root& root,
    const gltfjson::format::Bin& bin,
    std::optional<uint32_t> id)
  {
    if (!id) {
      return {};
    }

    auto found = m_materialMap.find(*id);
    if (found != m_materialMap.end()) {
      return found->second;
    }

    auto& src = root.Materials[*id];

    std::shared_ptr<grapho::gl3::Texture> albedo;
    std::shared_ptr<grapho::gl3::Texture> metallic;
    std::shared_ptr<grapho::gl3::Texture> roughness;
    if (auto pbr = src.PbrMetallicRoughness) {
      if (auto baseColorTexture = pbr->BaseColorTexture) {
        albedo = GetOrCreateTexture(
          root, bin, baseColorTexture->Index, libvrm::gltf::ColorSpace::sRGB);
      }
      if (auto metallicRoughnessTexture = pbr->MetallicRoughnessTexture) {
        metallic = GetOrCreateTexture(root,
                                      bin,
                                      metallicRoughnessTexture->Index,
                                      libvrm::gltf::ColorSpace::Linear);
        roughness = GetOrCreateTexture(root,
                                       bin,
                                       metallicRoughnessTexture->Index,
                                       libvrm::gltf::ColorSpace::Linear);
      }
    }
    std::shared_ptr<grapho::gl3::Texture> normal;
    if (auto normalTexture = src.NormalTexture) {
      normal = GetOrCreateTexture(
        root, bin, normalTexture->Index, libvrm::gltf::ColorSpace::Linear);
    }
    std::shared_ptr<grapho::gl3::Texture> ao;
    if (auto occlusionTexture = src.OcclusionTexture) {
      ao = GetOrCreateTexture(
        root, bin, occlusionTexture->Index, libvrm::gltf::ColorSpace::Linear);
    }

    auto material =
      grapho::gl3::PbrMaterial::Create(albedo, normal, metallic, roughness, ao);
    m_materialMap.insert({ *id, material });
    return material;
  }

  std::shared_ptr<grapho::gl3::Vao> GetOrCreate(
    const std::shared_ptr<runtimescene::BaseMesh>& mesh)
  {
    if (!mesh) {
      return {};
    }

    auto found = m_drawableMap.find(mesh);
    if (found != m_drawableMap.end()) {
      return found->second;
    }

    // load gpu resource
    auto vbo =
      grapho::gl3::Vbo::Create(mesh->verticesBytes(), mesh->m_vertices.data());
    auto ibo = grapho::gl3::Ibo::Create(
      mesh->indicesBytes(), mesh->m_indices.data(), GL_UNSIGNED_INT);

    std::shared_ptr<grapho::gl3::Vbo> slots[] = {
      vbo,
    };
    grapho::VertexLayout layouts[] = {
      {
        .Id = { 0, 0, "vPosition" },
        .Type = grapho::ValueType::Float,
        .Count = 3,
        .Offset = offsetof(runtimescene::Vertex, Position),
        .Stride = sizeof(runtimescene::Vertex),
      },
      {
        .Id = { 1, 0, "vNormal" },
        .Type = grapho::ValueType::Float,
        .Count = 3,
        .Offset = offsetof(runtimescene::Vertex, Normal),
        .Stride = sizeof(runtimescene::Vertex),
      },
      {
        .Id = { 2, 0, "vUv" },
        .Type = grapho::ValueType::Float,
        .Count = 2,
        .Offset = offsetof(runtimescene::Vertex, Uv),
        .Stride = sizeof(runtimescene::Vertex),
      },
    };
    auto vao = grapho::gl3::Vao::Create(layouts, slots, ibo);

    m_drawableMap.insert(std::make_pair(mesh, vao));

    return vao;
  }

  void Render(RenderPass pass,
              const RenderingEnv& env,
              const gltfjson::format::Root& root,
              const gltfjson::format::Bin& bin,
              const std::shared_ptr<runtimescene::BaseMesh>& mesh,
              const runtimescene::DeformedMesh& deformed,
              const DirectX::XMFLOAT4X4& m)
  {
    if (env.m_pbr) {
      env.m_pbr->Activate();
    }

    glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LESS);

    auto vao = GetOrCreate(mesh);

    if (deformed.Vertices.size()) {
      vao->slots_[0]->Upload(deformed.Vertices.size() *
                               sizeof(runtimescene::Vertex),
                             deformed.Vertices.data());
    }

    switch (pass) {
      case RenderPass::Color: {
        // m_program->Use();
        // m_program->Uniform("Projection")->SetMat4(env.ProjectionMatrix);
        // m_program->Uniform("View")->SetMat4(env.ViewMatrix);
        // m_program->Uniform("Model")->SetMat4(m);
        uint32_t drawOffset = 0;
        for (auto& primitive : mesh->m_primitives) {
          DrawPrimitive(env.ProjectionMatrix,
                        env.ViewMatrix,
                        m,
                        env.CameraPosition,
                        root,
                        bin,
                        vao,
                        primitive,
                        drawOffset);
          drawOffset += primitive.DrawCount * 4;
        }
        break;
      }

      case RenderPass::ShadowMatrix: {
        m_shadow->Use();
        m_shadow->Uniform("Projection")->SetMat4(env.ProjectionMatrix);
        m_shadow->Uniform("View")->SetMat4(env.ViewMatrix);
        m_shadow->Uniform("Shadow")->SetMat4(env.ShadowMatrix);
        m_shadow->Uniform("Model")->SetMat4(m);
        uint32_t drawCount = 0;
        for (auto& primitive : mesh->m_primitives) {
          drawCount += primitive.DrawCount * 4;
        }
        vao->Draw(GL_TRIANGLES, drawCount, 0);
        break;
      }
    }
  }

  static gltfjson::format::AlphaModes GetAlphaMode(
    const gltfjson::format::Root& root,
    std::optional<uint32_t> material)
  {
    if (material) {
      return root.Materials[*material].AlphaMode;
    } else {
      return gltfjson::format::AlphaModes::Opaque;
    }
  }

  void DrawPrimitive(const DirectX::XMFLOAT4X4& projection,
                     const DirectX::XMFLOAT4X4& view,
                     const DirectX::XMFLOAT4X4& model,
                     const DirectX::XMFLOAT3& cameraPos,
                     const gltfjson::format::Root& root,
                     const gltfjson::format::Bin& bin,
                     const std::shared_ptr<grapho::gl3::Vao>& vao,
                     const runtimescene::Primitive& primitive,
                     uint32_t drawOffset)
  {
    if (auto material = GetOrCreateMaterial(root, bin, primitive.Material)) {
      // auto texture = m_white;
      // if (auto t = primitive.Material->ColorTexture) {
      //   texture = GetOrCreate(t);
      // }
      material->Activate(projection,
                         view,
                         model,
                         cameraPos,
                         grapho::gl3::PbrEnv::UBO_LIGHTS_BINDING);

      // state
      glEnable(GL_CULL_FACE);
      glFrontFace(GL_CCW);
      glCullFace(GL_BGR);
      glEnable(GL_DEPTH_TEST);

      switch (auto alphaMode = GetAlphaMode(root, primitive.Material)) {
        case gltfjson::format::AlphaModes::Opaque:
          glDisable(GL_BLEND);
          break;
        case gltfjson::format::AlphaModes::Mask:
          glDisable(GL_BLEND);
          break;
        case gltfjson::format::AlphaModes::Blend:
          glEnable(GL_BLEND);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          break;
      }
      // m_program->Uniform("cutoff")->SetFloat(primitive.Material->AlphaCutoff);
      // m_program->Uniform("color")->SetFloat4(
      //   primitive.Material->Pbr.BaseColorFactor);

      // texture->Activate(0);
    } else {
      // default
    }
    vao->Draw(GL_TRIANGLES, primitive.DrawCount, drawOffset);
  }

  void CreateDock(const AddDockFunc& addDock, std::string_view title)
  {
    addDock(grapho::imgui::Dock(title, [this]() {
      for (auto it = m_drawableMap.begin(); it != m_drawableMap.end();) {
        if (it->first.lock()) {
          ++it;
        } else {
          // cleanup released
          it = m_drawableMap.erase(it);
        }
      }

      ImGui::Text("srgb: %zd(textures)", m_srgbTextureMap.size());
      ImGui::Text("linear: %zd(textures)", m_linearTextureMap.size());
      ImGui::SameLine();
      ImGui::Text("%zd(meshes)", m_drawableMap.size());
    }));
  }
};

void
Render(RenderPass pass,
       const RenderingEnv& env,
       const gltfjson::format::Root& root,
       const gltfjson::format::Bin& bin,
       const std::shared_ptr<runtimescene::BaseMesh>& mesh,
       const runtimescene::DeformedMesh& deformed,
       const DirectX::XMFLOAT4X4& m)
{
  Gl3Renderer::Instance().Render(pass, env, root, bin, mesh, deformed, m);
}

void
ClearRendertarget(const RenderingEnv& env)
{
  grapho::gl3::ClearViewport({
    .Width = env.Width(),
    .Height = env.Height(),
    .Color = { env.PremulR(), env.PremulG(), env.PremulB(), env.Alpha() },
    .Depth = 1.0f,
  });
  // glViewport(0, 0, env.Width(), env.Height());
  // glClearColor(env.PremulR(), env.PremulG(), env.PremulB(), env.Alpha());
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
Shutdown()
{
  Gl3Renderer::Instance().Release();
}

void
CreateDock(const AddDockFunc& addDock, std::string_view title)
{
  Gl3Renderer::Instance().CreateDock(addDock, title);
}

std::shared_ptr<grapho::gl3::Texture>
GetOrCreateTexture(const gltfjson::format::Root& root,
                   const gltfjson::format::Bin& bin,
                   std::optional<uint32_t> texture,
                   libvrm::gltf::ColorSpace colorspace)
{
  return Gl3Renderer::Instance().GetOrCreateTexture(
    root, bin, texture, colorspace);
}

void
RenderLine(const RenderingEnv& camera, std::span<const cuber::LineVertex> data)
{
  static cuber::gl3::GlLineRenderer s_liner;
  s_liner.Render(&camera.ProjectionMatrix._11, &camera.ViewMatrix._11, data);
}
}
