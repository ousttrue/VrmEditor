#pragma once
#include <functional>
#include <gltfjson.h>
#include <string>

using ShowGuiFunc = std::function<bool(const gltfjson::Root& root,
                                       const gltfjson::Bin& bin,
                                       const gltfjson::tree::NodePtr&)>;

using CreateGuiFunc = std::function<ShowGuiFunc(const gltfjson::tree::NodePtr&,
                                                std::u8string_view jsonpath)>;

struct JsonGuiItem
{
  std::u8string Icon;
  CreateGuiFunc Factory;
  std::string Description;
};

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

struct JsonProp
{
  std::u8string Key;
  std::u8string Icon;
  CreateGuiFunc Factory;
  JsonPropFlags Flags = JsonPropFlags::None;
  std::u8string Label() const { return Icon + Key; }
  std::u8string Value(const gltfjson::tree::NodePtr& item) const;
  ShowGuiFunc EditorOrDefault(const gltfjson::tree::NodePtr& item,
                              std::u8string_view jsonpath) const;
};

struct JsonObjectDefinition
{
  std::vector<JsonProp> Props;
};
