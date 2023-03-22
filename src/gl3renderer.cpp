#include "gl3renderer.h"
#include "camera.h"
#include "material.h"
#include "mesh.h"
#include <GL/glew.h>
#include <glo/shader.h>
#include <glo/texture.h>
#include <glo/vao.h>
#include <iostream>
#include <unordered_map>

static const char *vertex_shader_text = R"(#version 130
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
attribute vec3 vPosition;
attribute vec3 vNormal;
attribute vec2 vUv;
varying vec3 normal;
varying vec2 uv;
void main()
{
    gl_Position = Projection * View * Model * vec4(vPosition, 1.0);
    normal = vNormal;
    uv = vUv;
}
)";

static const char *fragment_shader_text = R"(#version 130
varying vec3 normal;
varying vec2 uv;
uniform sampler2D colorTexture;
void main()
{
    gl_FragColor = texture(colorTexture, uv);
};
)";

static GLenum indexType(int indexValueSize) {
  switch (indexValueSize) {
  case 1:
    return GL_UNSIGNED_BYTE;
  case 2:
    return GL_UNSIGNED_SHORT;
  case 4:
    return GL_UNSIGNED_INT;
  default:
    throw std::runtime_error("invalid value");
  }
}

struct SubMesh {
  uint32_t offset;
  uint32_t drawCount;
  std::shared_ptr<glo::Texture> texture;
};

struct Drawable {
  std::shared_ptr<glo::ShaderProgram> program;
  std::shared_ptr<glo::Vao> vao;
  std::vector<SubMesh> submeshes;

  void draw(const Camera &camera, const float m[16]) {
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

    for (auto &submesh : submeshes) {
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

class Gl3RendererImpl {
  std::unordered_map<uint32_t, std::shared_ptr<Drawable>> m_drawableMap;
  std::shared_ptr<glo::Texture> m_white;

public:
  Gl3RendererImpl() {
    std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GL_VENDOR: " << glGetString(GL_VENDOR) << std::endl;
    if (glewInit() != GLEW_OK) {
      throw std::runtime_error("glewInit");
    }

    static uint8_t white[] = {255, 255, 255, 255};
    m_white = glo::Texture::Create(1, 1, white);
  }

  ~Gl3RendererImpl() {}

  void clear(const Camera &camera) {
    glViewport(0, 0, camera.width(), camera.height());
    glClearColor(camera.premul_r(), camera.premul_g(), camera.premul_b(),
                 camera.alpha());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void render(const Camera &camera, const Mesh &mesh, const float m[16]) {
    auto drawable = getOrCreate(mesh);

    if (mesh.m_updated.size()) {
      drawable->vao->slots_[0].vbo->Upload(
          mesh.m_updated.size() * sizeof(Vertex), mesh.m_updated.data());
      // m_updated.clear();
    }

    drawable->draw(camera, m);
  }

  std::shared_ptr<Drawable> getOrCreate(const Mesh &mesh) {
    auto found = m_drawableMap.find(mesh.id);
    if (found != m_drawableMap.end()) {
      return found->second;
    }

    // load gpu resource
    auto vbo = glo::Vbo::Create(mesh.verticesBytes(), mesh.m_vertices.data());
    auto ibo = glo::Ibo::Create(mesh.m_indices.size(), mesh.m_indices.data(),
                                indexType(mesh.m_indexValueSize));

    glo::VertexLayout layouts[] = {
        {
            .id = {"vPosition", 0},
            .type = glo::ValueType::Float,
            .count = 3,
            .offset = offsetof(Vertex, position),
            .stride = sizeof(Vertex),
        },
        {
            .id = {"vNormal", 1},
            .type = glo::ValueType::Float,
            .count = 3,
            .offset = offsetof(Vertex, normal),
            .stride = sizeof(Vertex),
        },
        {
            .id = {"vUv", 0},
            .type = glo::ValueType::Float,
            .count = 2,
            .offset = offsetof(Vertex, uv),
            .stride = sizeof(Vertex),
        },
    };
    glo::VertexSlot slots[] = {
        {.location = 0, .vbo = vbo},
        {.location = 1, .vbo = vbo},
        {.location = 2, .vbo = vbo},
    };
    auto vao = glo::Vao::Create(layouts, slots, ibo);

    auto program =
        glo::ShaderProgram::Create(vertex_shader_text, fragment_shader_text);
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
    for (auto &submesh : mesh.m_primitives) {

      auto texture = m_white;
      if (auto image = submesh.material->texture) {
        texture = glo::Texture::Create(image->width(), image->height(),
                                       image->pixels());
      }

      drawable->submeshes.push_back({
          .offset = submesh.offset,
          .drawCount = submesh.drawCount,
          .texture = texture,
      });
    }

    m_drawableMap.insert(std::make_pair(mesh.id, drawable));

    return drawable;
  }
};

Gl3Renderer::Gl3Renderer() : m_impl(new Gl3RendererImpl) {}

Gl3Renderer::~Gl3Renderer() { delete m_impl; }

void Gl3Renderer::clear(const Camera &camera) { m_impl->clear(camera); }

void Gl3Renderer::render(const Camera &camera, const Mesh &mesh,
                         const float m[16]) {
  m_impl->render(camera, mesh, m);
}
