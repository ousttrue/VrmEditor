#pragma once
#include <DirectXMath.h>
#include <expected>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <type_traits>

namespace libvrm::gltf {
enum class ComponentType
{
  BYTE = 5120,
  UNSIGNED_BYTE = 5121,
  SHORT = 5122,
  UNSIGNED_SHORT = 5123,
  UNSIGNED_INT = 5125,
  FLOAT = 5126,
};
inline std::expected<size_t, std::string>
component_size(ComponentType component_type)
{
  switch (component_type) {
    case ComponentType::BYTE:
      return 1;
    case ComponentType::UNSIGNED_BYTE:
      return 1;
    case ComponentType::SHORT:
      return 2;
    case ComponentType::UNSIGNED_SHORT:
      return 2;
    case ComponentType::UNSIGNED_INT:
      return 4;
    case ComponentType::FLOAT:
      return 4;
    default:
      return std::unexpected{ "invalid component type" };
  }
}

enum class Type
{
  SCALAR,
  VEC2,
  VEC3,
  VEC4,
  MAT2,
  MAT3,
  MAT4,
};
inline std::expected<size_t, std::string>
type_size(std::string_view type)
{
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
    return std::unexpected{ std::string(type) };
  }
}
inline const char*
type_str(Type type)
{
  switch (type) {
    case Type::SCALAR:
      return "SCALAR";
    case Type::VEC2:
      return "VEC2";
    case Type::VEC3:
      return "VEC3";
    case Type::VEC4:
      return "VEC4";
    case Type::MAT2:
      return "MAT2";
    case Type::MAT3:
      return "MAT3";
    case Type::MAT4:
      return "MAT4";
    default:
      throw std::runtime_error("invalid");
  }
}

inline std::expected<size_t, std::string>
item_size(const nlohmann::json& accessor)
{
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

using WriteFunc = std::function<void(std::span<const uint8_t>)>;

struct BufferView
{
  uint32_t ByteOffset = 0;
  uint32_t ByteLength;
};

struct Accessor
{
  uint32_t ByteOffset = 0;
  uint32_t BufferView;
  uint32_t Count;
  Type Type;
  ComponentType ComponentType;

  template<typename T>
  static Accessor Create();
};

template<>
inline Accessor
Accessor::Create<uint8_t>()
{
  return Accessor{
    .Type = Type::SCALAR,
    .ComponentType = ComponentType::UNSIGNED_BYTE,
  };
}
template<>
inline Accessor
Accessor::Create<uint16_t>()
{
  return Accessor{
    .Type = Type::SCALAR,
    .ComponentType = ComponentType::UNSIGNED_SHORT,
  };
}
template<>
inline Accessor
Accessor::Create<uint32_t>()
{
  return Accessor{
    .Type = Type::SCALAR,
    .ComponentType = ComponentType::UNSIGNED_INT,
  };
}
template<>
inline Accessor
Accessor::Create<float>()
{
  return Accessor{
    .Type = Type::SCALAR,
    .ComponentType = ComponentType::FLOAT,
  };
}
template<>
inline Accessor
Accessor::Create<DirectX::XMFLOAT2>()
{
  return Accessor{
    .Type = Type::VEC2,
    .ComponentType = ComponentType::FLOAT,
  };
}
template<>
inline Accessor
Accessor::Create<DirectX::XMFLOAT3>()
{
  return Accessor{
    .Type = Type::VEC3,
    .ComponentType = ComponentType::FLOAT,
  };
}
template<>
inline Accessor
Accessor::Create<DirectX::XMFLOAT4>()
{
  return Accessor{
    .Type = Type::VEC4,
    .ComponentType = ComponentType::FLOAT,
  };
}

struct BinWriter
{
  WriteFunc m_write;

  std::vector<BufferView> BufferViews;
  std::vector<Accessor> Accessors;
  uint32_t BufferPosition = 0;

  BinWriter(const WriteFunc& writer)
    : m_write(writer)
  {
  }

  uint32_t PushBufferView(std::span<const uint8_t> values)
  {
    if (BufferPosition % 4) {
      // 4byte alignment
      auto size = 4 - (BufferPosition % 4);
      uint8_t padding[4] = { 0, 0, 0, 0 };
      m_write({ padding, size });
      BufferPosition += size;
    }

    m_write(values);
    auto index = BufferViews.size();
    BufferViews.push_back({
      .ByteOffset = BufferPosition,
      .ByteLength = static_cast<uint32_t>(values.size()),
    });
    BufferPosition += values.size();
    return index;
  }

  template<typename T>
  uint32_t PushAccessor(std::span<const T> values)
  {
    auto p = (const uint8_t*)values.data();
    auto bufferViewIndex = PushBufferView({ p, p + values.size() * sizeof(T) });
    auto index = Accessors.size();

    using TT = std::remove_const<T>::type;
    Accessors.push_back(Accessor::Create<TT>());
    Accessors.back().BufferView = bufferViewIndex,
    Accessors.back().Count = static_cast<uint32_t>(values.size());
    return index;
  }

  template<typename T>
  uint32_t PushAccessor(const std::vector<T> values)
  {
    return PushAccessor(std::span(values.data(), values.size()));
  }
};

}
