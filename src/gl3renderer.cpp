#include "gl3renderer.h"
#include <GL/glew.h>
#include <array>
#include <expected>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

using json = nlohmann::json;

struct float2 {
  float x;
  float y;
};
struct float3 {
  float x;
  float y;
  float z;
};
struct Vertex {
  float3 position;
  float3 normal;
  float2 uv;
};
static_assert(sizeof(Vertex) == 32, "sizeof(Vertex)");

struct Mesh {
  std::vector<Vertex> m_vertices;
  std::vector<uint8_t> m_indices;
  uint32_t m_drawCount = 0;
  GLenum indexType() const {
    switch (m_indices.size() / m_drawCount) {
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

  void setPosition(std::span<const float3> values) {
    std::cout << "position: " << values.size() << std::endl;
    m_vertices.resize(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
      m_vertices[i].position = values[i];
    }
  }

  void setNormal(std::span<const float3> values) {
    std::cout << "normal: " << values.size() << std::endl;
    m_vertices.resize(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
      m_vertices[i].normal = values[i];
    }
  }

  void setUv(std::span<const float2> values) {
    std::cout << "uv: " << values.size() << std::endl;
    m_vertices.resize(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
      m_vertices[i].uv = values[i];
    }
  }

  void setIndices(std::span<const uint8_t> values, size_t count) {
    m_indices.assign(values.begin(), values.end());
    m_drawCount = count;
  }
};

// static const struct {
//   float x, y;
//   float r, g, b;
// } vertices[3] = {{-0.6f, -0.4f, 1.f, 0.f, 0.f},
//                  {0.6f, -0.4f, 0.f, 1.f, 0.f},
//                  {0.f, 0.6f, 0.f, 0.f, 1.f}};

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

class Gl3RendererImpl {
  GLuint vertex_buffer = 0;
  GLuint ibo_buffer = 0;
  GLuint vertex_shader = 0;
  GLuint fragment_shader = 0;
  GLuint program = 0;
  GLint projection_location = -1;
  GLint view_location = -1;
  GLint vpos_location = -1;
  GLint vcol_location = -1;

  uint32_t m_drawCount = 0;
  uint32_t m_indexType = 0;

public:
  Gl3RendererImpl() {
    std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GL_VENDOR: " << glGetString(GL_VENDOR) << std::endl;
    if (glewInit() != GLEW_OK) {
      throw std::runtime_error("glewInit");
    }

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
  }

  ~Gl3RendererImpl() {}

  void render(const Camera &camera) {
    glViewport(0, 0, camera.width(), camera.height());
    glClearColor(camera.premul_r(), camera.premul_g(), camera.premul_b(),
                 camera.alpha());
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, camera.projection);
    glUniformMatrix4fv(view_location, 1, GL_FALSE, camera.view);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_buffer);
    // glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawElements(GL_TRIANGLES, m_drawCount, m_indexType, nullptr);
  }

  void loadMesh(const Mesh &mesh) {
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mesh.m_vertices.size(),
                 mesh.m_vertices.data(), GL_STATIC_DRAW);
    // vertex layout
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, uv));

    glGenBuffers(1, &ibo_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.m_indices.size(),
                 mesh.m_indices.data(), GL_STATIC_DRAW);

    m_drawCount = mesh.m_drawCount;
    m_indexType = mesh.indexType();
  }
};

Gl3Renderer::Gl3Renderer() : m_impl(new Gl3RendererImpl) {}

Gl3Renderer::~Gl3Renderer() { delete m_impl; }

void Gl3Renderer::render(const Camera &camera) { m_impl->render(camera); }

template <typename T>
static std::vector<T> ReadAllBytes(const std::string &filename) {
  std::ifstream ifs(filename.c_str(), std::ios::binary | std::ios::ate);
  if (!ifs) {
    return {};
  }
  auto pos = ifs.tellg();
  auto size = pos / sizeof(T);
  if (pos % sizeof(T)) {
    ++size;
  }
  std::vector<T> buffer(size);
  ifs.seekg(0, std::ios::beg);
  ifs.read((char *)buffer.data(), pos);
  return buffer;
}

class BinaryReader {
  std::span<const uint8_t> m_data;
  size_t m_pos = 0;

public:
  BinaryReader(std::span<const uint8_t> data) : m_data(data) {}

  template <typename T> T get() {
    auto value = *((T *)&m_data[m_pos]);
    m_pos += sizeof(T);
    return value;
  }

  void resize(size_t len) { m_data = std::span(m_data.begin(), len); }

  std::span<const uint8_t> span(size_t size) {
    auto value = m_data.subspan(m_pos, size);
    m_pos += size;
    return value;
  }

  std::string_view string_view(size_t size) {
    auto value = m_data.subspan(m_pos, size);
    m_pos += size;
    return std::string_view((const char *)value.data(),
                            (const char *)value.data() + value.size());
  }

  bool is_end() const { return m_pos >= m_data.size(); }
};

static std::expected<size_t, std::string> component_size(int component_type) {
  switch (component_type) {
  case 5120: // BYTE
    return 1;
  case 5121: // UNSIGNED_BYTE
    return 1;
  case 5122: // SHORT
    return 2;
  case 5123: // UNSIGNED_SHORT
    return 2;
  case 5125: // UNSIGNED_INT
    return 4;
  case 5126: // FLOAT
    return 4;
  default:
    return std::unexpected{"invalid component type"};
  }
}

static std::expected<size_t, std::string> type_size(std::string_view type) {
  if (type == "SCALAR") {
    return 1;
  } else if (type == "VEC2") {
    return 2;
  } else if (type == "VEC3") {
    return 3;
  } else if (type == "VEC4") {
    return 4;
  } else if (type == "MAT2") {
    return 4;
  } else if (type == "MAT3") {
    return 9;
  } else if (type == "MAT4") {
    return 16;
  } else {
    return std::unexpected{std::string(type)};
  }
}

static std::expected<size_t, std::string> item_size(const json &accessor) {
  if (auto cs = component_size(accessor["componentType"])) {
    if (auto ts = type_size(accessor["type"])) {
      return *cs * *ts;
    } else {
      return ts;
    }
  } else {
    throw cs;
  }
}

struct Glb {
  json gltf;
  std::span<const uint8_t> bin;

  std::span<const uint8_t> buffer_view(int buffer_view_index) {
    auto buffer_view = gltf["bufferViews"][buffer_view_index];
    // std::cout << buffer_view << std::endl;
    return bin.subspan(buffer_view["byteOffset"], buffer_view["byteLength"]);
  }

  template <typename T> std::span<const T> accessor(int accessor_index) {
    auto accessor = gltf["accessors"][accessor_index];
    std::cout << accessor << std::endl;
    assert(*item_size(accessor) == sizeof(T));
    auto span = buffer_view(accessor["bufferView"]);
    return std::span<const T>((const T *)span.data(), accessor["count"]);
  }

  std::tuple<std::span<const uint8_t>, uint32_t> indices(int accessor_index) {
    auto accessor = gltf["accessors"][accessor_index];
    std::cout << accessor << std::endl;
    auto span = buffer_view(accessor["bufferView"]);
    return {span, accessor["count"]};
  }
};
std::optional<Glb> parseGlb(std::span<const uint8_t> bytes) {

  BinaryReader r(bytes);
  if (r.get<uint32_t>() != 0x46546C67) {
    return {};
  }

  if (r.get<uint32_t>() != 2) {
    return {};
  }

  auto length = r.get<uint32_t>();
  r.resize(length);

  Glb glb{};
  {
    auto chunk_length = r.get<uint32_t>();
    if (r.get<uint32_t>() != 0x4E4F534A) {
      // first chunk must "JSON"
      return {};
    }
    auto gltf = r.string_view(chunk_length);
    glb.gltf = json::parse(gltf);
  }
  if (!r.is_end()) {
    auto chunk_length = r.get<uint32_t>();
    if (r.get<uint32_t>() != 0x004E4942) {
      // second chunk is "BIN"
      return {};
    }
    glb.bin = r.span(chunk_length);
  }

  return glb;
}

void Gl3Renderer::load(const char *path) {
  auto bytes = ReadAllBytes<uint8_t>(path);
  if (bytes.empty()) {
    return;
  }

  auto glb = parseGlb(bytes);
  if (!glb) {
    return;
  }

  auto meshes = glb->gltf["meshes"];
  auto mesh = meshes[0];
  for (auto prim : mesh["primitives"]) {
    Mesh buffer;

    json attributes = prim["attributes"];
    for (auto &kv : attributes.items()) {
      auto key = kv.key();
      // std::cout << key << ": " << kv.value() << std::endl;
      if (kv.key() == "POSITION") {
        auto values = glb->accessor<float3>(kv.value());
        buffer.setPosition(values);
      } else if (kv.key() == "NORMAL") {
        auto values = glb->accessor<float3>(kv.value());
        buffer.setNormal(values);
      } else if (kv.key() == "TEXCOORD_0") {
        auto values = glb->accessor<float2>(kv.value());
        buffer.setUv(values);
      } else {
        throw std::runtime_error(kv.key() + " is not implemened");
      }
    }
    {
      std::cout << "indices: " << prim["indices"] << std::endl;
      auto [span, draw_count] = glb->indices(prim["indices"]);
      buffer.setIndices(span, draw_count);
    }
    m_impl->loadMesh(buffer);

    break;
  }
}
