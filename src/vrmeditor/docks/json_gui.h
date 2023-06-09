#pragma once
#include "printfbuffer.h"
#include <functional>
#include <gltfjson.h>
#include <gltfjson/jsonpath.h>
#include <string>
#include <vrm/gltfroot.h>

using ShowGuiFunc = std::function<bool(const gltfjson::Root& root,
                                       const gltfjson::Bin& bin,
                                       const gltfjson::tree::NodePtr&)>;

using CreateGuiFunc = std::function<ShowGuiFunc(std::u8string_view jsonpath)>;

struct JsonGuiItem
{
  std::u8string Icon;
  CreateGuiFunc Factory;
  std::string Description;
};

enum class JsonPropFlags : uint32_t
{
  None = 0,
  Requried = 0x01,
  ReadOnly = 0x02,
  Unknown = 0x04,
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
  std::u8string Value(const gltfjson::tree::NodePtr& item) const
  {
    if (item) {
      std::stringstream ss;
      ss << *item;
      auto str = ss.str();
      return std::u8string{ (const char8_t*)str.data(), str.size() };
    } else {
      return u8"";
    }
  }
};

struct JsonObjectDefinition
{
  std::vector<JsonProp> Props;
};

struct JsonGui
{
  std::shared_ptr<libvrm::GltfRoot> m_root;
  gltfjson::JsonPathMap<JsonObjectDefinition> m_definitionMap;
  PrintfBuffer m_buf;
  std::u8string m_jsonpath;

  struct Cache
  {
    std::u8string Label;
    std::u8string value;
    ShowGuiFunc Editor;
  };
  std::unordered_map<std::u8string, Cache> m_cacheMap;

  JsonGui();
  void ClearCache();
  void SetScene(const std::shared_ptr<libvrm::GltfRoot>& root);
  bool Enter(const gltfjson::tree::NodePtr& item,
             const std::u8string& jsonpath,
             const JsonProp& prop);
  void ShowSelector(float indent);

private:
  void Traverse(const gltfjson::tree::NodePtr& item,
                std::u8string& jsonpath,
                const JsonProp& prop);

  bool ShouldOpen(std::u8string_view jsonpath) const
  {
    if (jsonpath.size() == 1) {
      // "/"
      return true;
    }

    if (m_jsonpath.starts_with(jsonpath)) {
      if (m_jsonpath[jsonpath.size()] == u8'/') {
        return true;
      }
    }
    return false;
  }
};
