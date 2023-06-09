#pragma once
// #include "../printfbuffer.h"
#include "jsonschema/json_prop.h"
#include <functional>
#include <gltfjson.h>
#include <gltfjson/jsonpath.h>
#include <string>
#include <vrm/gltfroot.h>

enum class EditorResult
{
  None,
  Updated,
  KeyCreated,
  Removed,
};

struct JsonGui
{
  std::shared_ptr<libvrm::GltfRoot> m_root;
  gltfjson::JsonPathMap<JsonSchema> m_definitionMap;
  // PrintfBuffer m_buf;
  std::u8string m_jsonpath;

  struct Cache
  {
    std::u8string Label;
    std::u8string Value;
    ShowGuiFunc Editor;
    std::function<void()> ShowTag;
  };
  std::unordered_map<std::u8string, Cache> m_cacheMap;

  JsonGui();
  void ClearCache(const std::u8string& jsonpath = {});
  void SetScene(const std::shared_ptr<libvrm::GltfRoot>& root);
  void ShowSelector(float indent);

private:
  EditorResult Traverse(const gltfjson::tree::NodePtr& item,
                        std::u8string& jsonpath,
                        const JsonProp& prop);

  std::tuple<bool, EditorResult> Enter(const gltfjson::tree::NodePtr& item,
                                       const std::u8string& jsonpath,
                                       const JsonProp& prop);

  bool OnEdit(const gltfjson::tree::NodePtr& parent,
              const gltfjson::tree::NodePtr& item,
              const std::u8string& jsonpath,
              const JsonProp& prop,
              EditorResult result);

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
