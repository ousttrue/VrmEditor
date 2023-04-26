#include "gl3renderer.h"
#include "rendering_env.h"
#include <GL/glew.h>
#include <cuber/gl3/GlLineRenderer.h>
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
#include <vrm/texture.h>

static const char* vertex_shader_text = R"(#version 400
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

static const char* fragment_shader_text = R"(#version 400
in vec3 normal;
in vec2 uv;
out vec4 FragColor;
uniform sampler2D colorTexture;
void main()
{
  vec4 texel = texture(colorTexture, uv);
  FragColor = vec4(texel.rgb, 1);
};
)";

static const char* shadow_vertex_text = R"(#version 400
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

static const char* shadow_fragment_text = R"(#version 400
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
  using ImageWeakPtr = std::weak_ptr<libvrm::gltf::Image>;
  using MeshWeakPtr = std::weak_ptr<libvrm::gltf::Mesh>;

  std::map<ImageWeakPtr,
           std::shared_ptr<grapho::gl3::Texture>,
           std::owner_less<ImageWeakPtr>>
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
    m_white = grapho::gl3::Texture::Create(1, 1, white);

    m_program = *grapho::gl3::ShaderProgram::Create(vertex_shader_text,
                                                    fragment_shader_text);
    m_shadow = *grapho::gl3::ShaderProgram::Create(shadow_vertex_text,
                                                   shadow_fragment_text);
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
    const std::shared_ptr<libvrm::gltf::Image>& image)
  {
    auto found = m_textureMap.find(image);
    if (found != m_textureMap.end()) {
      return found->second;
    }
    auto texture = grapho::gl3::Texture::Create(
      image->Width(), image->Height(), image->Pixels());
    m_textureMap.insert(std::make_pair(image, texture));
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

    grapho::VertexLayout layouts[] = {
      {
        .id = { "vPosition", 0 },
        .type = grapho::ValueType::Float,
        .count = 3,
        .offset = offsetof(Vertex, Position),
        .stride = sizeof(Vertex),
      },
      {
        .id = { "vNormal", 1 },
        .type = grapho::ValueType::Float,
        .count = 3,
        .offset = offsetof(Vertex, Normal),
        .stride = sizeof(Vertex),
      },
      {
        .id = { "vUv", 0 },
        .type = grapho::ValueType::Float,
        .count = 2,
        .offset = offsetof(Vertex, Uv),
        .stride = sizeof(Vertex),
      },
    };
    grapho::gl3::VertexSlot slots[] = {
      { .location = 0, .vbo = vbo },
      { .location = 1, .vbo = vbo },
      { .location = 2, .vbo = vbo },
    };
    auto vao = grapho::gl3::Vao::Create(layouts, slots, ibo);

    m_drawableMap.insert(std::make_pair(mesh, vao));

    return vao;
  }

  void Render(RenderPass pass,
              const RenderingEnv& env,
              const std::shared_ptr<libvrm::gltf::Mesh>& mesh,
              const libvrm::gltf::MeshInstance& instance,
              const float m[16])
  {
    auto vao = GetOrCreate(mesh);

    if (instance.m_updated.size()) {
      vao->slots_[0].vbo->Upload(instance.m_updated.size() * sizeof(Vertex),
                                 instance.m_updated.data());
    }

    switch (pass) {
      case RenderPass::Color:
        m_program->Bind();
        m_program->SetUniformMatrix("Projection", env.ProjectionMatrix);
        m_program->SetUniformMatrix("View", env.ViewMatrix);
        m_program->_SetUniformMatrix("Model", m);
        break;

      case RenderPass::ShadowMatrix:
        m_shadow->Bind();
        m_shadow->SetUniformMatrix("Projection", env.ProjectionMatrix);
        m_shadow->SetUniformMatrix("View", env.ViewMatrix);
        m_shadow->SetUniformMatrix("Shadow", env.ShadowMatrix);
        m_shadow->_SetUniformMatrix("Model", m);
        break;
    }

    uint32_t drawOffset = 0;
    for (auto& primitive : mesh->m_primitives) {
      if (auto material = primitive.Material) {
        auto texture = m_white;
        if (auto t = primitive.Material->ColorTexture) {
          if (auto image = t->Source) {
            texture = GetOrCreate(image);
          }
        }

        // state
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BGR);
        glEnable(GL_DEPTH_TEST);

        switch (material->AlphaBlend) {
          case libvrm::gltf::BlendMode::Opaque:
            glDisable(GL_BLEND);
            break;
          case libvrm::gltf::BlendMode::Mask:
            glDisable(GL_BLEND);
            break;
          case libvrm::gltf::BlendMode::Blend:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            // glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            // glBlendFunc(GL_ONE, GL_ZERO);
            break;
        }

        texture->Bind(0);
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

      ImGui::Text("%zd", m_drawableMap.size());
    }));
  }
};

void
Render(RenderPass pass,
       const RenderingEnv& env,
       const std::shared_ptr<libvrm::gltf::Mesh>& mesh,
       const libvrm::gltf::MeshInstance& instance,
       const float m[16])
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
GetOrCreate(const std::shared_ptr<libvrm::gltf::Image>& image)
{
  return Gl3Renderer::Instance().GetOrCreate(image);
}

void
RenderLine(const RenderingEnv& camera, std::span<const grapho::LineVertex> data)
{
  static cuber::gl3::GlLineRenderer s_liner;
  s_liner.Render(camera.ProjectionMatrix, camera.ViewMatrix, data);
}

}
