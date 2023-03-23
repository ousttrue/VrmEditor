#include "vrm/glb.h"

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
std::optional<Glb> Glb::parse(std::span<const uint8_t> bytes) {

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
