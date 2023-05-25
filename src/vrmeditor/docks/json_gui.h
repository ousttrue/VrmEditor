#pragma once
// #include "gui.h"
#include <gltfjson.h>
// #include <gltfjson/jsonpath.h>
// #include <sstream>
#include <string>
// #include <unordered_map>
// #include <vector>

class LabelCacheManager;
class JsonGuiFactoryManager;
struct JsonGui
{
  gltfjson::tree::NodePtr m_gltf;
  std::shared_ptr<LabelCacheManager> m_label;
  std::shared_ptr<JsonGuiFactoryManager> m_inspector;

  JsonGui();
  void SetScene(const gltfjson::tree::NodePtr& gltf);
  bool Enter(const gltfjson::tree::NodePtr& item, std::u8string_view jsonpath);
  void ShowSelector(float indent);
  void ShowSelected();
};
