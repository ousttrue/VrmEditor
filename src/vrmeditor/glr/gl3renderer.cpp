#include "gl3renderer.h"
#include "app.h"
#include "rendering_env.h"
#include "shader_source.h"
#include <DirectXMath.h>
#include <GL/glew.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <cuber/mesh.h>
#include <grapho/gl3/pbr.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/texture.h>
#include <grapho/gl3/ubo.h>
#include <grapho/gl3/vao.h>
#include <imgui.h>
#include <iostream>
#include <unordered_map>
#include <variant>
#include <vrm/deformed_mesh.h>
#include <vrm/fileutil.h>
#include <vrm/gltf.h>
#include <vrm/image.h>

namespace glr {

static gltfjson::format::AlphaModes
GetAlphaMode(const gltfjson::typing::Root& root,
             std::optional<uint32_t> material)
{
  if (material) {
    if (auto alphaMode = root.Materials[*material].AlphaMode()) {
      return (gltfjson::format::AlphaModes)*alphaMode;
    }
  }
  return gltfjson::format::AlphaModes::Opaque;
}

static std::expected<std::shared_ptr<libvrm::gltf::Image>, std::string>
ParseImage(const gltfjson::typing::Root& root,
           const gltfjson::typing::Bin& bin,
           const gltfjson::typing::Image& image)
{
  std::span<const uint8_t> bytes;
  if (auto bufferView = image.BufferView()) {
    if (auto buffer_view = bin.GetBufferViewBytes(root, *bufferView)) {
      bytes = *buffer_view;
    } else {
      return std::unexpected{ buffer_view.error() };
    }
  } else if (image.Uri().size()) {
    if (auto buffer_view = bin.Dir->GetBuffer(image.Uri())) {
      bytes = *buffer_view;
    } else {
      return std::unexpected{ buffer_view.error() };
    }
  } else {
    return std::unexpected{ "not bufferView nor uri" };
  }
  auto name = image.Name();
  auto ptr = std::make_shared<libvrm::gltf::Image>(name);
  if (!ptr->Load(bytes)) {
    return std::unexpected{ "Image: fail to load" };
  }
  return ptr;
}

class Gl3Renderer
{
  std::unordered_map<uint32_t, std::shared_ptr<libvrm::gltf::Image>> m_imageMap;
  std::unordered_map<uint32_t, std::shared_ptr<grapho::gl3::Texture>>
    m_srgbTextureMap;
  std::unordered_map<uint32_t, std::shared_ptr<grapho::gl3::Texture>>
    m_linearTextureMap;
  std::unordered_map<uint32_t, std::shared_ptr<grapho::gl3::Material>>
    m_materialMap;
  std::unordered_map<uint32_t, std::shared_ptr<grapho::gl3::Vao>> m_drawableMap;

  std::shared_ptr<grapho::gl3::Texture> m_white;
  std::shared_ptr<grapho::gl3::ShaderProgram> m_shadow;
  std::shared_ptr<grapho::gl3::ShaderProgram> m_error;

  ShaderSourceManager m_shaderSource;

  grapho::gl3::Material::EnvVars m_env = {};
  std::shared_ptr<grapho::gl3::Ubo> m_envUbo;
  grapho::gl3::Material::ModelVars m_model = {};
  std::shared_ptr<grapho::gl3::Ubo> m_modelUbo;

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

    m_envUbo = grapho::gl3::Ubo::Create<grapho::gl3::Material::EnvVars>();
    m_modelUbo = grapho::gl3::Ubo::Create<grapho::gl3::Material::ModelVars>();
  }

  ~Gl3Renderer() {}

public:
  static Gl3Renderer& Instance()
  {
    static Gl3Renderer s_instance;
    return s_instance;
  }

  void Release()
  {
    m_imageMap.clear();
    m_materialMap.clear();
    m_srgbTextureMap.clear();
    m_linearTextureMap.clear();
    m_materialMap.clear();
    m_drawableMap.clear();
  }

  // for local shader
  void SetShaderDir(const std::filesystem::path& path)
  {
    m_shaderSource.SetShaderDir(path);
    // clear cache
    m_materialMap.clear();
    m_shadow = {};
  }

  // for hot reload
  // use relative path. pbr.{vs,fs}, unlit.{vs,fs}, mtoon.{vs,fs}
  void UpdateShader(const std::filesystem::path& path)
  {
    auto str = path.string();
    if (str.starts_with("pbr.")) {
      // clear cache
      m_materialMap.clear();
    } else if (str.starts_with("shadow.")) {
      m_shadow = {};
    }
    m_shaderSource.UpdateShader(path);
  }

  std::shared_ptr<libvrm::gltf::Image> GetOrCreateImage(
    const gltfjson::typing::Root& root,
    const gltfjson::typing::Bin& bin,
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
    const gltfjson::typing::Root& root,
    const gltfjson::typing::Bin& bin,
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

    auto src = root.Textures[*id];
    auto source = src.Source();
    if (!source) {
      return {};
    }

    auto image = GetOrCreateImage(root, bin, *source);

    auto texture = grapho::gl3::Texture::Create({
      image->Width(),
      image->Height(),
      grapho::PixelFormat::u8_RGBA,
      colorspace == libvrm::gltf::ColorSpace::sRGB ? grapho::ColorSpace::sRGB
                                                   : grapho::ColorSpace::Linear,
      image->Pixels(),
    });

    if (auto samplerIndex = src.Sampler()) {
      auto sampler = root.Samplers[*samplerIndex];
      texture->Bind();
      glTexParameteri(GL_TEXTURE_2D,
                      GL_TEXTURE_MAG_FILTER,
                      gltfjson::typing::value_or<int>(
                        sampler.MagFilter(),
                        (int)gltfjson::format::TextureMagFilter::LINEAR));
      glTexParameteri(GL_TEXTURE_2D,
                      GL_TEXTURE_MIN_FILTER,
                      gltfjson::typing::value_or<int>(
                        sampler.MinFilter(),
                        (int)gltfjson::format::TextureMinFilter::LINEAR));
      glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        gltfjson::typing::value_or<int>(
          sampler.WrapS(), (int)gltfjson::format::TextureWrap::REPEAT));
      glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        gltfjson::typing::value_or<int>(
          sampler.WrapT(), (int)gltfjson::format::TextureWrap::REPEAT));
      texture->Unbind();
    } else {
      // TODO: default sampler
    }

    map.insert(std::make_pair(*id, texture));
    return texture;
  }

  std::shared_ptr<grapho::gl3::Material> GetOrCreateMaterial(
    const gltfjson::typing::Root& root,
    const gltfjson::typing::Bin& bin,
    std::optional<uint32_t> id)
  {
    if (!id) {
      return {};
    }

    auto found = m_materialMap.find(*id);
    if (found != m_materialMap.end()) {
      return found->second;
    }

    auto src = root.Materials[*id];

    // auto unlit =
    //   std::find_if(src.Extensions.begin(), src.Extensions.end(), [](auto&
    //   ex)
    //   {
    //     return ex.Name == u8"KHR_materials_unlit";
    //   });
    // if (unlit != src.Extensions.end()) {
    if (false) {
      //
      // unlit
      //
      if (auto shader = grapho::gl3::ShaderProgram::Create(
            m_shaderSource.Get("unlit.vert"),
            m_shaderSource.Get("unlit.frag"))) {

        auto material = std::make_shared<grapho::gl3::Material>();
        material->Shader = *shader;
        if (auto pbr = src.PbrMetallicRoughness()) {
          if (auto baseColorTexture = pbr->BaseColorTexture()) {
            if (auto texture =
                  GetOrCreateTexture(root,
                                     bin,
                                     baseColorTexture->Index(),
                                     libvrm::gltf::ColorSpace::sRGB)) {
              material->Textures.push_back({ 0, texture });
            }
          }
        }
        auto inserted = m_materialMap.insert({ *id, material });
        return inserted.first->second;
      } else {
        App::Instance().Log(LogLevel::Error) << "unlit: " << shader.error();
      }
    } else {
      //
      // PBR
      //
      std::shared_ptr<grapho::gl3::Texture> albedo;
      std::shared_ptr<grapho::gl3::Texture> metallic;
      std::shared_ptr<grapho::gl3::Texture> roughness;
      if (auto pbr = src.PbrMetallicRoughness()) {
        if (auto baseColorTexture = pbr->BaseColorTexture()) {
          albedo = GetOrCreateTexture(root,
                                      bin,
                                      baseColorTexture->Index(),
                                      libvrm::gltf::ColorSpace::sRGB);
        }
        if (auto metallicRoughnessTexture = pbr->MetallicRoughnessTexture()) {
          metallic = GetOrCreateTexture(root,
                                        bin,
                                        metallicRoughnessTexture->Index(),
                                        libvrm::gltf::ColorSpace::Linear);
          roughness = GetOrCreateTexture(root,
                                         bin,
                                         metallicRoughnessTexture->Index(),
                                         libvrm::gltf::ColorSpace::Linear);
        }
      }
      std::shared_ptr<grapho::gl3::Texture> normal;
      if (auto normalTexture = src.NormalTexture()) {
        normal = GetOrCreateTexture(
          root, bin, normalTexture->Index(), libvrm::gltf::ColorSpace::Linear);
      }
      std::shared_ptr<grapho::gl3::Texture> ao;
      if (auto occlusionTexture = src.OcclusionTexture()) {
        ao = GetOrCreateTexture(root,
                                bin,
                                occlusionTexture->Index(),
                                libvrm::gltf::ColorSpace::Linear);
      }

      auto vs = m_shaderSource.Get("pbr.vert");
      auto fs = m_shaderSource.Get("pbr.frag");
      if (auto material = grapho::gl3::CreatePbrMaterial(
            albedo, normal, metallic, roughness, ao, vs, fs)) {
        auto inserted = m_materialMap.insert({ *id, *material });
        return inserted.first->second;
      } else {
        App::Instance().Log(LogLevel::Error) << "pbr: " << material.error();
      }
    }

    m_materialMap.insert({ *id, {} });
    return {};
  }

  std::shared_ptr<grapho::gl3::Vao> GetOrCreateMesh(
    uint32_t id,
    const std::shared_ptr<runtimescene::BaseMesh>& mesh)
  {
    assert(mesh);

    auto found = m_drawableMap.find(id);
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

    m_drawableMap.insert({ id, vao });

    return vao;
  }

  void Render(RenderPass pass,
              const RenderingEnv& env,
              const gltfjson::typing::Root& root,
              const gltfjson::typing::Bin& bin,
              uint32_t meshId,
              const std::shared_ptr<runtimescene::BaseMesh>& mesh,
              const runtimescene::DeformedMesh& deformed,
              const DirectX::XMFLOAT4X4& m)
  {
    if (env.m_pbr) {
      env.m_pbr->Activate();
    }

    glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LESS);

    auto vao = GetOrCreateMesh(meshId, mesh);

    if (deformed.Vertices.size()) {
      vao->slots_[0]->Upload(deformed.Vertices.size() *
                               sizeof(runtimescene::Vertex),
                             deformed.Vertices.data());
    }

    switch (pass) {
      case RenderPass::Color: {
        m_env.projection = env.ProjectionMatrix;
        m_env.view = env.ViewMatrix;
        m_env.camPos = {
          env.CameraPosition.x,
          env.CameraPosition.y,
          env.CameraPosition.z,
          1,
        };
        m_envUbo->Upload(m_env);
        m_envUbo->SetBindingPoint(0);
        m_model.model = m;
        m_model.CalcNormalMatrix();
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
        if (!m_shadow) {
          if (auto shadow = grapho::gl3::ShaderProgram::Create(
                m_shaderSource.Get("shadow.vert"),
                m_shaderSource.Get("shadow.frag"))) {
            m_shadow = *shadow;
          } else {
            App::Instance().Log(LogLevel::Error) << shadow.error();
          }
        }
        if (m_shadow) {
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
        }
        break;
      }
    }
  }

  void DrawPrimitive(const DirectX::XMFLOAT4X4& projection,
                     const DirectX::XMFLOAT4X4& view,
                     const DirectX::XMFLOAT4X4& model,
                     const DirectX::XMFLOAT3& cameraPos,
                     const gltfjson::typing::Root& root,
                     const gltfjson::typing::Bin& bin,
                     const std::shared_ptr<grapho::gl3::Vao>& vao,
                     const runtimescene::Primitive& primitive,
                     uint32_t drawOffset)
  {
    auto material = GetOrCreateMaterial(root, bin, primitive.Material);
    if (material) {
      // update ubo
      auto gltfMaterial = root.Materials[*primitive.Material];
      if (auto cutoff = gltfMaterial.AlphaCutoff()) {
        m_model.cutoff.x = *cutoff;
      }
      if (auto pbr = gltfMaterial.PbrMetallicRoughness()) {
        // m_model.color = *((DirectX::XMFLOAT4*)&pbr->BaseColorFactor());
      }
      m_modelUbo->Upload(m_model);
      m_modelUbo->SetBindingPoint(1);

      material->Activate();

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
      // error
      if (!m_error) {
        if (auto error = grapho::gl3::ShaderProgram::Create(
              m_shaderSource.Get("error.vert"),
              m_shaderSource.Get("error.frag"))) {
          m_error = *error;
        }
      }
      if (m_error) {
        m_error->Use();
        m_error->Uniform("Projection")->SetMat4(projection);
        m_error->Uniform("View")->SetMat4(view);
        m_error->Uniform("Model")->SetMat4(model);
      }
    }
    vao->Draw(GL_TRIANGLES, primitive.DrawCount, drawOffset);
  }

  void CreateDock(const AddDockFunc& addDock, std::string_view title)
  {
    addDock(grapho::imgui::Dock(title, [this]() {
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
       const gltfjson::typing::Root& root,
       const gltfjson::typing::Bin& bin,
       uint32_t meshId,
       const std::shared_ptr<runtimescene::BaseMesh>& mesh,
       const runtimescene::DeformedMesh& deformed,
       const DirectX::XMFLOAT4X4& m)
{
  Gl3Renderer::Instance().Render(
    pass, env, root, bin, meshId, mesh, deformed, m);
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
Release()
{
  Gl3Renderer::Instance().Release();
}

void
CreateDock(const AddDockFunc& addDock, std::string_view title)
{
  Gl3Renderer::Instance().CreateDock(addDock, title);
}

std::shared_ptr<grapho::gl3::Texture>
GetOrCreateTexture(const gltfjson::typing::Root& root,
                   const gltfjson::typing::Bin& bin,
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

// for local shader
void
SetShaderDir(const std::filesystem::path& path)
{
  Gl3Renderer::Instance().SetShaderDir(path);
}

// for hot reload
// use relative path. pbr.{vs,fs}, unlit.{vs,fs}, mtoon.{vs,fs}
void
UpdateShader(const std::filesystem::path& path)
{
  Gl3Renderer::Instance().UpdateShader(path);
}
}
