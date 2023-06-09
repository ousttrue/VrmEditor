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
  JsonPropFlags Flags = JsonPropFlags::None;
};

struct JsonObjectDefinition
{
  std::vector<JsonProp> Props;
};

class JsonGuiFactoryManager;
struct JsonGui
{
  std::shared_ptr<libvrm::GltfRoot> m_root;
  std::shared_ptr<JsonGuiFactoryManager> m_inspector;
  gltfjson::JsonPathMap<JsonObjectDefinition> m_definitionMap;
  PrintfBuffer m_buf;

  std::unordered_map<std::u8string, uint32_t> m_idMap;

  JsonGui();
  void SetScene(const std::shared_ptr<libvrm::GltfRoot>& root);
  bool Enter(const gltfjson::tree::NodePtr& item,
             const std::u8string& jsonpath,
             const JsonProp& prop);
  void ShowSelector(float indent);
  void ShowSelected();

private:
  void Traverse(const gltfjson::tree::NodePtr& item,
                std::u8string& jsonpath,
                const JsonProp& prop);
};
