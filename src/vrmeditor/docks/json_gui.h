#pragma once
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
  CreateGuiFunc Editor;
};

class LabelCacheManager;
class JsonGuiFactoryManager;
struct JsonGui
{
  std::shared_ptr<libvrm::GltfRoot> m_root;
  std::shared_ptr<LabelCacheManager> m_label;
  std::shared_ptr<JsonGuiFactoryManager> m_inspector;

  JsonGui();
  void SetScene(const std::shared_ptr<libvrm::GltfRoot>& root);
  bool Enter(const gltfjson::tree::NodePtr& item, std::u8string_view jsonpath);
  void ShowSelector(float indent);
  void ShowSelected();
};
