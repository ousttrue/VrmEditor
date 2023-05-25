#pragma once
#include <gltfjson.h>
#include <string>
#include <vrm/gltfroot.h>

class LabelCacheManager;
class JsonGuiFactoryManager;
struct JsonGui
{
  std::shared_ptr<libvrm::gltf::GltfRoot> m_root;
  std::shared_ptr<LabelCacheManager> m_label;
  std::shared_ptr<JsonGuiFactoryManager> m_inspector;

  JsonGui();
  void SetScene(const std::shared_ptr<libvrm::gltf::GltfRoot>& root);
  bool Enter(const gltfjson::tree::NodePtr& item, std::u8string_view jsonpath);
  void ShowSelector(float indent);
  void ShowSelected();
};
