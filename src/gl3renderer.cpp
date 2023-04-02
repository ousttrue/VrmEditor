#include "gl3renderer.h"
#include "camera.h"
#include <GL/glew.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/texture.h>
#include <grapho/gl3/vao.h>
#include <iostream>
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

  void draw(const Camera& camera, const float m[16])
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

class Gl3RendererImpl
{
  std::unordered_map<uint32_t, std::shared_ptr<Drawable>> m_drawableMap;
  std::shared_ptr<grapho::gl3::Texture> m_white;

public:
  Gl3RendererImpl()
  {
    static uint8_t white[] = { 255, 255, 255, 255 };
    m_white = grapho::gl3::Texture::Create(1, 1, white);
  }

  ~Gl3RendererImpl() {}

  void clear(const Camera& camera)
  {
    glViewport(0, 0, camera.width(), camera.height());
    glClearColor(
      camera.premul_r(), camera.premul_g(), camera.premul_b(), camera.alpha());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void render(const Camera& camera,
              const gltf::Mesh& mesh,
              const gltf::MeshInstance& instance,
              const float m[16])
  {
    auto drawable = getOrCreate(mesh);

    if (instance.m_updated.size()) {
      drawable->vao->slots_[0].vbo->Upload(
        instance.m_updated.size() * sizeof(Vertex), instance.m_updated.data());
      // m_updated.clear();
    }

    drawable->draw(camera, m);
  }

  std::shared_ptr<Drawable> getOrCreate(const gltf::Mesh& mesh)
  {
    auto found = m_drawableMap.find(mesh.id);
    if (found != m_drawableMap.end()) {
      return found->second;
    }

    // load gpu resource
    auto vbo =
      grapho::gl3::Vbo::Create(mesh.verticesBytes(), mesh.m_vertices.data());
    auto ibo = grapho::gl3::Ibo::Create(
      mesh.indicesBytes(), mesh.m_indices.data(), GL_UNSIGNED_INT);

    grapho::VertexLayout layouts[] = {
      {
        .id = { "vPosition", 0 },
        .type = grapho::ValueType::Float,
        .count = 3,
        .offset = offsetof(Vertex, position),
        .stride = sizeof(Vertex),
      },
      {
        .id = { "vNormal", 1 },
        .type = grapho::ValueType::Float,
        .count = 3,
        .offset = offsetof(Vertex, normal),
        .stride = sizeof(Vertex),
      },
      {
        .id = { "vUv", 0 },
        .type = grapho::ValueType::Float,
        .count = 2,
        .offset = offsetof(Vertex, uv),
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
    for (auto& primitive : mesh.m_primitives) {

      auto texture = m_white;
      if (auto image = primitive.material->texture) {
        texture = grapho::gl3::Texture::Create(
          image->width(), image->height(), image->pixels());
      }

      drawable->submeshes.push_back({
        .offset = byteOffset,
        .drawCount = primitive.drawCount,
        .texture = texture,
      });
      // GL_UNSIGNED_INT
      byteOffset += primitive.drawCount * 4;
    }

    m_drawableMap.insert(std::make_pair(mesh.id, drawable));

    return drawable;
  }
};

Gl3Renderer::Gl3Renderer()
  : m_impl(new Gl3RendererImpl)
{
}

Gl3Renderer::~Gl3Renderer()
{
  delete m_impl;
}

void
Gl3Renderer::clear(const Camera& camera)
{
  m_impl->clear(camera);
}

void
Gl3Renderer::render(const Camera& camera,
                    const gltf::Mesh& mesh,
                    const gltf::MeshInstance& instance,
                    const float m[16])
{
  m_impl->render(camera, mesh, instance, m);
}
