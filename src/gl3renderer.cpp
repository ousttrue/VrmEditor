#include "gl3renderer.h"
#include <GL/glew.h>
#include <array>
#include <span>
#include <stdexcept>

static const struct {
  float x, y;
  float r, g, b;
} vertices[3] = {{-0.6f, -0.4f, 1.f, 0.f, 0.f},
                 {0.6f, -0.4f, 0.f, 1.f, 0.f},
                 {0.f, 0.6f, 0.f, 0.f, 1.f}};

static const char *vertex_shader_text = R"(#version 110
uniform mat4 View;
uniform mat4 Projection;
attribute vec3 vCol;
attribute vec2 vPos;
varying vec3 color;
void main()
{
    gl_Position = Projection * View * vec4(vPos, 0.0, 1.0);
    color = vCol;
}
)";

static const char *fragment_shader_text = R"(#version 110
varying vec3 color;
void main()
{
    gl_FragColor = vec4(color, 1.0);
};
)";

GLuint vertex_buffer, vertex_shader, fragment_shader, program;
GLint projection_location;
GLint view_location;
GLint vpos_location, vcol_location;

Gl3Renderer::Gl3Renderer() {

  glewInit();

  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
  glCompileShader(vertex_shader);

  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
  glCompileShader(fragment_shader);

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  projection_location = glGetUniformLocation(program, "Projection");
  view_location = glGetUniformLocation(program, "View");
  vpos_location = glGetAttribLocation(program, "vPos");
  vcol_location = glGetAttribLocation(program, "vCol");

  glEnableVertexAttribArray(vpos_location);
  glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]), (void *)0);
  glEnableVertexAttribArray(vcol_location);
  glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]), (void *)(sizeof(float) * 2));
}

Gl3Renderer::~Gl3Renderer() {}

void Gl3Renderer::render(const Camera &camera) {
  glViewport(0, 0, camera.width(), camera.height());
  glClearColor(camera.premul_r(), camera.premul_g(), camera.premul_b(),
               camera.alpha());
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(program);
  glUniformMatrix4fv(projection_location, 1, GL_FALSE, camera.projection);
  glUniformMatrix4fv(view_location, 1, GL_FALSE, camera.view);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}
