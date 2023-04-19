#include "vrm/glb.h"
#include <assert.h>
#include <fstream>

class BinaryReader
{
  std::span<const uint8_t> m_data;
  size_t m_pos = 0;

public:
  BinaryReader(std::span<const uint8_t> data)
    : m_data(data)
  {
  }

  template<typename T>
  T Get()
  {
    auto value = *((T*)&m_data[m_pos]);
    m_pos += sizeof(T);
    return value;
  }

  void Resize(size_t len) { m_data = std::span(m_data.begin(), len); }

  std::span<const uint8_t> Span(size_t size)
  {
    auto value = m_data.subspan(m_pos, size);
    m_pos += size;
    return value;
  }

  std::string_view StringView(size_t size)
  {
    auto value = m_data.subspan(m_pos, size);
    m_pos += size;
    return std::string_view((const char*)value.data(),
                            (const char*)value.data() + value.size());
  }

  bool is_end() const { return m_pos >= m_data.size(); }
};

class BinaryWriter
{
  std::ostream& m_os;

public:
  BinaryWriter(std::ostream& os)
    : m_os(os)
  {
  }

  void Uint32(uint32_t value) { m_os.write((const char*)&value, 4); }
  void Bytes(std::span<const uint8_t> values)
  {
    m_os.write((const char*)values.data(), values.size());
  }
  void Padding(uint32_t size)
  {
    assert(size < 4);
    uint8_t padding[4] = { 0, 0, 0, 0 };
    Bytes({ padding, size });
  }
};

namespace libvrm::gltf {

const uint32_t GLB_MAGIC = 0x46546C67;
const uint32_t GLB_VERSION = 2;
const uint32_t GLB_JSON_CHUNK = 0x4E4F534A;
const uint32_t GLB_BIN_CHUNK = 0x004E4942;

std::optional<Glb>
Glb::Parse(std::span<const uint8_t> bytes)
{
  BinaryReader r(bytes);
  if (r.Get<uint32_t>() != GLB_MAGIC) {
    return {};
  }

  if (r.Get<uint32_t>() != GLB_VERSION) {
    return {};
  }

  auto length = r.Get<uint32_t>();
  r.Resize(length);

  Glb glb{};
  {
    auto chunk_length = r.Get<uint32_t>();
    if (r.Get<uint32_t>() != GLB_JSON_CHUNK) {
      // first chunk must "JSON"
      return {};
    }
    glb.JsonChunk = r.Span(chunk_length);
  }
  if (!r.is_end()) {
    auto chunk_length = r.Get<uint32_t>();
    if (r.Get<uint32_t>() != GLB_BIN_CHUNK) {
      // second chunk is "BIN"
      return {};
    }
    glb.BinChunk = r.Span(chunk_length);
  }

  return glb;
}

bool
Glb::WriteTo(std::ostream &os)
{
  BinaryWriter w(os);

  // GLB header
  w.Uint32(GLB_MAGIC);
  w.Uint32(GLB_VERSION);
  w.Uint32(CalcSize());

  // json
  w.Uint32(JsonChunk.size() + JsonPadding());
  w.Uint32(GLB_JSON_CHUNK);
  w.Bytes(JsonChunk);
  w.Padding(JsonPadding());

  // bin
  w.Uint32(BinChunk.size() + BinPadding());
  w.Uint32(GLB_BIN_CHUNK);
  w.Bytes(BinChunk);
  w.Padding(BinPadding());

  return true;
}

}
