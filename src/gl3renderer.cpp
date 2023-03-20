#include "gl3renderer.h"
#include "camera.h"
#include "mesh.h"
#include <GL/glew.h>
#include <glo/shader.h>
#include <glo/vao.h>
#include <iostream>
#include <unordered_map>

static const char *vertex_shader_text = R"(#version 110
uniform mat4 View;
uniform mat4 Projection;
attribute vec3 vPosition;
attribute vec3 vNormal;
attribute vec2 vUv;
varying vec3 normal;
varying vec2 uv;
void main()
{
    gl_Position = Projection * View * vec4(vPosition, 1.0);
    normal = vNormal;
    uv = vUv;
}
)";

static const char *fragment_shader_text = R"(#version 110
varying vec3 normal;
varying vec2 uv;
void main()
{
    gl_FragColor = vec4(normal, 1.0);
};
)";

static void printError(std::string_view msg) { std::cout << msg << std::endl; }

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

struct Drawable {
  std::shared_ptr<glo::ShaderProgram> program;
  std::shared_ptr<glo::Vao> vao;
  uint32_t drawCount = 0;

  void draw(const Camera &camera) {
    // state
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BGR);
    glEnable(GL_DEPTH_TEST);
    // mesh
    program->Bind();
    program->SetUniformMatrix(printError, "Projection", camera.projection);
    program->SetUniformMatrix(printError, "View", camera.view);
    vao->Draw(GL_TRIANGLES, drawCount);
  }
};

class Gl3RendererImpl {
  std::unordered_map<uint32_t, Drawable> m_drawableMap;

public:
  Gl3RendererImpl() {
    std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GL_VENDOR: " << glGetString(GL_VENDOR) << std::endl;
    if (glewInit() != GLEW_OK) {
      throw std::runtime_error("glewInit");
    }
  }

  ~Gl3RendererImpl() {}

  void clear(const Camera &camera) {
    glViewport(0, 0, camera.width(), camera.height());
    glClearColor(camera.premul_r(), camera.premul_g(), camera.premul_b(),
                 camera.alpha());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void render(const Camera &camera, const Mesh &mesh) {
    auto drawable = getOrCreate(mesh);
    drawable.draw(camera);
  }

  Drawable getOrCreate(const Mesh &mesh) {
    auto found = m_drawableMap.find(mesh.id);
    if (found != m_drawableMap.end()) {
      return found->second;
    }

    // load gpu resource
    auto vbo = glo::Vbo::Create(mesh.verticesBytes(), mesh.m_vertices.data());
    auto ibo = glo::Ibo::Create(mesh.m_indices.size(), mesh.m_indices.data(),
                                indexType(mesh.indexValueSize()));

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

    auto drawCount = mesh.m_drawCount;

    auto program = glo::ShaderProgram::Create(printError, vertex_shader_text,
                                              fragment_shader_text);

    // projection_location = *m_program->UniformLocation("Projection");
    // view_location = *m_program->UniformLocation("View");
    // std::cout << "shader OK" << std::endl;

    return Drawable{
        .program = program,
        .vao = vao,
        .drawCount = drawCount,
    };
  }
};

Gl3Renderer::Gl3Renderer() : m_impl(new Gl3RendererImpl) {}

Gl3Renderer::~Gl3Renderer() { delete m_impl; }

void Gl3Renderer::clear(const Camera &camera) { m_impl->clear(camera); }

void Gl3Renderer::render(const Camera &camera, const Mesh &mesh) {
  m_impl->render(camera, mesh);
}
