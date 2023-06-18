#pragma once
#include <functional>
#include <gltfjson.h>
#include <string>

using ShowGuiFunc = std::function<bool(const gltfjson::Root& root,
                                       const gltfjson::Bin& bin,
                                       const gltfjson::tree::NodePtr&)>;

using ShowTagFunc = std::function<void()>;
using TagFunc = std::function<ShowTagFunc(const gltfjson::Root& root,
                                          const gltfjson::Bin& bin,
                                          const gltfjson::tree::NodePtr& item)>;

struct NameObject
{
  std::u8string Icon;
  std::u8string Key;
  std::u8string Label() const { return Icon + Key; }
};

enum class JsonPropFlags : uint32_t
{
  None = 0,
  Required = 0x01,
  Lock = 0x02,
  ArrayAdd = 0x04,
  NoRemove = 0x08,
};
inline JsonPropFlags
operator|(JsonPropFlags lhs, JsonPropFlags rhs)
{
  return static_cast<JsonPropFlags>((int)lhs | (int)rhs);
}
inline JsonPropFlags
operator&(JsonPropFlags lhs, JsonPropFlags rhs)
{
  return static_cast<JsonPropFlags>((int)lhs & (int)rhs);
}
inline bool
Has(JsonPropFlags lhs, JsonPropFlags rhs)
{
  return (int)lhs & (int)rhs;
}

enum class JsonValueFlags : uint32_t
{
  None = 0,
  FixedArray = 0x01,
  ShowDefaultIfNone = 0x02,
};

struct JsonValue
{
  ShowGuiFunc Editor;
  std::u8string DefaultJson;
  JsonValueFlags Flags = JsonValueFlags::None;
  std::u8string TextOrDeault(const gltfjson::tree::NodePtr& item) const;
  ShowGuiFunc EditorOrDefault() const;

  const static JsonValue True;
  const static JsonValue False;
  const static JsonValue Number;
  const static JsonValue String;
  const static JsonValue Object;
  const static JsonValue Array;
  const static JsonValue Vec3;
  const static JsonValue Vec4;
  const static JsonValue Mat4;
};

inline const std::u8string
U8Q(const char* str)
{
  return std::u8string(u8"\"") + std::u8string((const char8_t*)str) +
         std::u8string(u8"\"");
}

inline const JsonValue JsonValue::True{ {}, u8"true" };
inline const JsonValue JsonValue::False{ {}, u8"false" };
inline const JsonValue JsonValue::Number{ {}, u8"0" };
inline const JsonValue JsonValue::String{ {}, U8Q("") };
inline const JsonValue JsonValue::Object{ {}, u8"{}" };
inline const JsonValue JsonValue::Array{ {}, u8"[]" };
inline const JsonValue JsonValue::Vec3{ {},
                                        u8"[0,0,0]",
                                        JsonValueFlags::FixedArray };
inline const JsonValue JsonValue::Vec4{ {},
                                        u8"[0,0,0,0]",
                                        JsonValueFlags::FixedArray };
inline const JsonValue JsonValue::Mat4{ {},
                                        u8"[0,0,0,0"
                                        u8",0,0,0,0"
                                        u8",0,0,0,0"
                                        u8",0,0,0,0]",
                                        JsonValueFlags::FixedArray };

struct JsonProp
{
  NameObject Name;
  JsonValue Value;
  JsonPropFlags Flags = JsonPropFlags::None;
  TagFunc Tag;
};

struct JsonSchema
{
  std::vector<JsonProp> Props;
};
