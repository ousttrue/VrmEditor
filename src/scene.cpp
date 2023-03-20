#include "scene.h"
#include "mesh.h"
#include <expected>
#include <fstream>
#include <iostream>
#include <list>
#include <nlohmann/json.hpp>
#include <span>
#include <string>
#include <vector>

using json = nlohmann::json;

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

struct SceneImpl {
  std::list<std::shared_ptr<Mesh>> m_meshes;
};

Scene::Scene() : m_impl(new SceneImpl) {}
Scene::~Scene() { delete m_impl; }

void Scene::load(const char *path) {
  auto bytes = ReadAllBytes<uint8_t>(path);
  if (bytes.empty()) {
    return;
  }

  auto glb = parseGlb(bytes);
  if (!glb) {
    return;
  }

  for (auto &mesh : glb->gltf["meshes"]) {
    for (auto prim : mesh["primitives"]) {
      auto buffer = std::make_shared<Mesh>();

      json attributes = prim["attributes"];
      for (auto &kv : attributes.items()) {
        auto key = kv.key();
        // std::cout << key << ": " << kv.value() << std::endl;
        if (kv.key() == "POSITION") {
          auto values = glb->accessor<float3>(kv.value());
          buffer->setPosition(values);
        } else if (kv.key() == "NORMAL") {
          auto values = glb->accessor<float3>(kv.value());
          buffer->setNormal(values);
        } else if (kv.key() == "TEXCOORD_0") {
          auto values = glb->accessor<float2>(kv.value());
          buffer->setUv(values);
        } else {
          throw std::runtime_error(kv.key() + " is not implemened");
        }
      }
      {
        std::cout << "indices: " << prim["indices"] << std::endl;
        auto [span, draw_count] = glb->indices(prim["indices"]);
        buffer->setIndices(span, draw_count);
      }

      m_impl->m_meshes.push_back(buffer);
    }
  }
}

void Scene::render(const Camera &camera, const RenderFunc &render) {
  for (auto &mesh : m_impl->m_meshes) {
    render(camera, *mesh);
  }
}
