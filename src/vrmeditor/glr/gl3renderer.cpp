#include "gl3renderer.h"
#include "viewporjection.h"
#include <GL/glew.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/texture.h>
#include <grapho/gl3/vao.h>
#include <imgui.h>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vrm/material.h>
#include <vrm/mesh.h>

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

namespace glr {
struct SubMesh
{
  uint32_t offset;
  uint32_t drawCount;
  std::shared_ptr<grapho::gl3::Texture> texture;
};

struct Drawable
{
  std::shared_ptr<grapho::gl3::ShaderProgram> program;
  std::shared_ptr<grapho::gl3::Vao> vao;
  std::vector<SubMesh> submeshes;

  void draw(const ViewProjection& camera, const float m[16])
  {
    // state
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BGR);
    glEnable(GL_DEPTH_TEST);
    // mesh
    program->Bind();
    program->SetUniformMatrix("Projection", camera.projection);
    program->SetUniformMatrix("View", camera.view);
    program->_SetUniformMatrix("Model", m);

    for (auto& submesh : submeshes) {
      // setup material
      if (submesh.texture) {
        submesh.texture->Bind();
      } else {
        // dummy white texture
      }
      vao->Draw(GL_TRIANGLES, submesh.drawCount, submesh.offset);
    }
  }
};

class Gl3Renderer
{
  // https://stackoverflow.com/questions/12875652/how-can-i-use-a-stdmap-with-stdweak-ptr-as-key
  using ImageWeakPtr = std::weak_ptr<libvrm::gltf::Image>;
  using MeshWeakPtr = std::weak_ptr<libvrm::gltf::Mesh>;

  std::map<ImageWeakPtr,
           std::shared_ptr<grapho::gl3::Texture>,
           std::owner_less<ImageWeakPtr>>
    m_textureMap;
  std::map<MeshWeakPtr, std::shared_ptr<Drawable>, std::owner_less<MeshWeakPtr>>
    m_drawableMap;
  std::shared_ptr<grapho::gl3::Texture> m_white;

  Gl3Renderer()
  {
    static uint8_t white[] = { 255, 255, 255, 255 };
    m_white = grapho::gl3::Texture::Create(1, 1, white);
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

  std::shared_ptr<Drawable> GetOrCreate(
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

    auto program = grapho::gl3::ShaderProgram::Create(vertex_shader_text,
                                                      fragment_shader_text);
    if (!program) {
      std::cout << program.error() << std::endl;
      return {};
    }

    // projection_location = *m_program->UniformLocation("Projection");
    // view_location = *m_program->UniformLocation("View");
    // std::cout << "shader OK" << std::endl;

    auto drawable = std::shared_ptr<Drawable>(new Drawable);
    drawable->program = *program;
    drawable->vao = vao;

    uint32_t byteOffset = 0;
    for (auto& primitive : mesh->m_primitives) {

      auto texture = m_white;
      if (auto image = primitive.material->Texture) {
        texture = GetOrCreate(image);
      }

      drawable->submeshes.push_back({
        .offset = byteOffset,
        .drawCount = primitive.drawCount,
        .texture = texture,
      });
      // GL_UNSIGNED_INT
      byteOffset += primitive.drawCount * 4;
    }

    m_drawableMap.insert(std::make_pair(mesh, drawable));

    return drawable;
  }

  void Render(const ViewProjection& camera,
              const std::shared_ptr<libvrm::gltf::Mesh>& mesh,
              const libvrm::gltf::MeshInstance& instance,
              const float m[16])
  {
    auto drawable = GetOrCreate(mesh);

    if (instance.m_updated.size()) {
      drawable->vao->slots_[0].vbo->Upload(
        instance.m_updated.size() * sizeof(Vertex), instance.m_updated.data());
      // m_updated.clear();
    }

    drawable->draw(camera, m);
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
Render(const ViewProjection& camera,
       const std::shared_ptr<libvrm::gltf::Mesh>& mesh,
       const libvrm::gltf::MeshInstance& instance,
       const float m[16])
{
  Gl3Renderer::Instance().Render(camera, mesh, instance, m);
}

void
ClearRendertarget(const ViewProjection& camera)
{
  glViewport(0, 0, camera.width(), camera.height());
  glClearColor(
    camera.premul_r(), camera.premul_g(), camera.premul_b(), camera.alpha());
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

}
