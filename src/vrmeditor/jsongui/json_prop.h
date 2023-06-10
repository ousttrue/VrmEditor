#pragma once
#include <functional>
#include <gltfjson.h>
#include <string>

using ShowGuiFunc = std::function<bool(const gltfjson::Root& root,
                                       const gltfjson::Bin& bin,
                                       const gltfjson::tree::NodePtr&)>;

enum class JsonPropFlags : uint32_t
{
  None = 0,
  Required = 0x01,
  ReadOnly = 0x02,
  Unknown = 0x04,
  ArrayChild = 0x08,
  DictObject = 0x10,
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

using ValueTextFunc =
  std::function<std::u8string(const gltfjson::tree::NodePtr& item)>;

struct JsonValue
{
  // active node editor
  ShowGuiFunc Editor;
  // texture represents or default value
  ValueTextFunc Text;
  // default json node for add
  std::u8string DefaultJson = u8"no default";

  std::u8string TextOrDeault(const gltfjson::tree::NodePtr& item) const
  {
    if (item) {
      if (Text) {
        return Text(item);
      } else {
        return u8"default text func";
      }
    } else {
      return DefaultJson;
    }
  }
  ShowGuiFunc EditorOrDefault() const;
};

struct JsonProp
{
  std::u8string Key;
  std::u8string Icon;
  JsonValue Value;
  JsonPropFlags Flags = JsonPropFlags::None;
  std::u8string Label() const { return Icon + Key; }
  // ShowGuiFunc EditorOrDefault(const gltfjson::tree::NodePtr& item,
  //                             std::u8string_view jsonpath) const;
};

struct JsonObjectDefinition
{
  std::vector<JsonProp> Props;
};
