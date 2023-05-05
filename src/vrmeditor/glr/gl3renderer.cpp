#include "gl3renderer.h"
#include "app.h"
#include "rendering_env.h"
#include <DirectXMath.h>
#include <GL/glew.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <cuber/mesh.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/texture.h>
#include <grapho/gl3/vao.h>
#include <imgui.h>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vrm/image.h>
#include <vrm/material.h>
#include <vrm/mesh.h>
#include <vrm/runtimescene/mesh.h>
#include <vrm/texture.h>

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

class Gl3Renderer
{
  // https://stackoverflow.com/questions/12875652/how-can-i-use-a-stdmap-with-stdweak-ptr-as-key
  // using ImageWeakPtr = std::weak_ptr<libvrm::gltf::Image>;
  using TextureWeakPtr = std::weak_ptr<libvrm::gltf::Texture>;
  using MaterialWeakPtr = std::weak_ptr<libvrm::gltf::Material>;
  using MeshWeakPtr = std::weak_ptr<libvrm::gltf::Mesh>;

  std::map<TextureWeakPtr,
           std::shared_ptr<grapho::gl3::Texture>,
           std::owner_less<TextureWeakPtr>>
    m_textureMap;

  std::map<MeshWeakPtr,
           std::shared_ptr<grapho::gl3::Vao>,
           std::owner_less<MeshWeakPtr>>
    m_drawableMap;

  std::shared_ptr<grapho::gl3::Texture> m_white;
  std::shared_ptr<grapho::gl3::ShaderProgram> m_program;
  std::shared_ptr<grapho::gl3::ShaderProgram> m_shadow;

  Gl3Renderer()
  {
    static uint8_t white[] = { 255, 255, 255, 255 };
    m_white =
      grapho::gl3::Texture::Create(1, 1, grapho::PixelFormat::u8_RGBA, white);

    if (auto program = grapho::gl3::ShaderProgram::Create(
          vertex_shader_text, fragment_shader_text)) {
      m_program = *program;
    } else {
      App::Instance().Log(LogLevel::Error) << program.error();
    }
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

  std::shared_ptr<grapho::gl3::Texture> GetOrCreate(
    const std::shared_ptr<libvrm::gltf::Texture>& src)
  {
    auto found = m_textureMap.find(src);
    if (found != m_textureMap.end()) {
      return found->second;
    }

    auto texture = grapho::gl3::Texture::Create(src->Source->Width(),
                                                src->Source->Height(),
                                                grapho::PixelFormat::u8_RGBA,
                                                src->Source->Pixels());
    m_textureMap.insert(std::make_pair(src, texture));
    return texture;
  }

  std::shared_ptr<grapho::gl3::Vao> GetOrCreate(
    const std::shared_ptr<libvrm::gltf::Mesh>& mesh)
  {
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
        .Offset = offsetof(libvrm::Vertex, Position),
        .Stride = sizeof(libvrm::Vertex),
      },
      {
        .Id = { 1, 0, "vNormal" },
        .Type = grapho::ValueType::Float,
        .Count = 3,
        .Offset = offsetof(libvrm::Vertex, Normal),
        .Stride = sizeof(libvrm::Vertex),
      },
      {
        .Id = { 2, 0, "vUv" },
        .Type = grapho::ValueType::Float,
        .Count = 2,
        .Offset = offsetof(libvrm::Vertex, Uv),
        .Stride = sizeof(libvrm::Vertex),
      },
    };
    auto vao = grapho::gl3::Vao::Create(layouts, slots, ibo);

    m_drawableMap.insert(std::make_pair(mesh, vao));

    return vao;
  }

  void Render(RenderPass pass,
              const RenderingEnv& env,
              const std::shared_ptr<libvrm::gltf::Mesh>& mesh,
              const runtimescene::RuntimeMesh& instance,
              const DirectX::XMFLOAT4X4& m)
  {
    if (!m_program) {
      return;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    auto vao = GetOrCreate(mesh);

    if (instance.m_updated.size()) {
      vao->slots_[0]->Upload(instance.m_updated.size() * sizeof(libvrm::Vertex),
                             instance.m_updated.data());
    }

    switch (pass) {
      case RenderPass::Color:
        m_program->Use();
        m_program->Uniform("Projection")->SetMat4(env.ProjectionMatrix);
        m_program->Uniform("View")->SetMat4(env.ViewMatrix);
        m_program->Uniform("Model")->SetMat4(m);
        break;

      case RenderPass::ShadowMatrix:
        m_shadow->Use();
        m_shadow->Uniform("Projection")->SetMat4(env.ProjectionMatrix);
        m_shadow->Uniform("View")->SetMat4(env.ViewMatrix);
        m_shadow->Uniform("Shadow")->SetMat4(env.ShadowMatrix);
        m_shadow->Uniform("Model")->SetMat4(m);
        break;
    }

    uint32_t drawOffset = 0;
    for (auto& primitive : mesh->m_primitives) {
      if (auto material = primitive.Material) {
        auto texture = m_white;
        if (auto t = primitive.Material->ColorTexture) {
          texture = GetOrCreate(t);
        }

        // state
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BGR);
        glEnable(GL_DEPTH_TEST);

        switch (material->AlphaBlendMode) {
          case libvrm::gltf::BlendMode::Opaque:
            glDisable(GL_BLEND);
            break;
          case libvrm::gltf::BlendMode::Mask:
            glDisable(GL_BLEND);
            break;
          case libvrm::gltf::BlendMode::Blend:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        }
        m_program->Uniform("cutoff")->SetFloat(material->AlphaCutoff);
        m_program->Uniform("color")->SetFloat4(material->Color);

        texture->Activate(0);
      }

      vao->Draw(GL_TRIANGLES, primitive.DrawCount, drawOffset);
      drawOffset += primitive.DrawCount * 4;
    }
  }

  void CreateDock(const AddDockFunc& addDock, std::string_view title)
  {
    addDock(Dock(title, [this]() {
      for (auto it = m_drawableMap.begin(); it != m_drawableMap.end();) {
        if (it->first.lock()) {
          ++it;
        } else {
          // cleanup released
          it = m_drawableMap.erase(it);
        }
      }

      ImGui::Text("%zd(textures)", m_textureMap.size());
      ImGui::SameLine();
      ImGui::Text("%zd(meshes)", m_drawableMap.size());
    }));
  }
};

void
Render(RenderPass pass,
       const RenderingEnv& env,
       const std::shared_ptr<libvrm::gltf::Mesh>& mesh,
       const runtimescene::RuntimeMesh& instance,
       const DirectX::XMFLOAT4X4& m)
{
  Gl3Renderer::Instance().Render(pass, env, mesh, instance, m);
}

void
ClearRendertarget(const RenderingEnv& env)
{
  glViewport(0, 0, env.Width(), env.Height());
  glClearColor(env.PremulR(), env.PremulG(), env.PremulB(), env.Alpha());
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
GetOrCreate(const std::shared_ptr<libvrm::gltf::Texture>& texture)
{
  return Gl3Renderer::Instance().GetOrCreate(texture);
}

void
RenderLine(const RenderingEnv& camera, std::span<const cuber::LineVertex> data)
{
  static cuber::gl3::GlLineRenderer s_liner;
  s_liner.Render(camera.ProjectionMatrix, camera.ViewMatrix, data);
}

}
