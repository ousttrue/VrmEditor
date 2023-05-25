#pragma once
#include "gui.h"
#include <gltfjson.h>
#include <gltfjson/jsonpath.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <vrm/gltfroot.h>

using ShowGui = std::function<void()>;

using CreateGuiFunc =
  std::function<ShowGui(const std::shared_ptr<libvrm::gltf::GltfRoot>& scene,
                        std::u8string_view jsonpath)>;
struct JsonGuiFactory
{
  std::u8string m_match;
  CreateGuiFunc Factory;

  bool Match(std::u8string_view jsonpath)
  {
    return gltfjson::JsonPath(m_match).Match(jsonpath);
  }
};

using CreateLabelFunc = std::function<std::string(
  const std::shared_ptr<libvrm::gltf::GltfRoot>& scene,
  std::u8string_view jsonpath)>;

struct LabelCache
{
  std::string Key;
  std::string Value;
};

struct JsonLabelFactory
{
  std::u8string m_match;
  CreateLabelFunc Factory;

  bool Match(std::u8string_view jsonpath)
  {
    return gltfjson::JsonPath(m_match).Match(jsonpath);
  }
};

class LabelCacheManager
{
  std::shared_ptr<libvrm::gltf::GltfRoot> m_scene;
  std::list<JsonLabelFactory> m_labelFactories;
  std::unordered_map<std::u8string, LabelCache> m_labelCache;

public:
  LabelCacheManager();
  std::optional<CreateLabelFunc> MatchLabel(std::u8string_view jsonpath)
  {
    for (auto& f : m_labelFactories) {
      if (f.Match(jsonpath)) {
        return f.Factory;
      }
    }
    // not found
    return {};
  }

  const LabelCache& Get(const gltfjson::tree::NodePtr& item,
                        std::u8string_view jsonpath);

  void SetScene(const std::shared_ptr<libvrm::gltf::GltfRoot>& scene)
  {
    m_scene = scene;
    m_labelCache.clear();
  }
};

struct JsonGui
{
  std::shared_ptr<libvrm::gltf::GltfRoot> m_scene;
  std::u8string m_selected;

  std::list<JsonGuiFactory> m_guiFactories;
  ShowGui m_cache;

  LabelCacheManager m_label;

  JsonGui();

  void SetScene(const std::shared_ptr<libvrm::gltf::GltfRoot>& scene)
  {
    m_scene = scene;
    m_cache = {};
    m_label.SetScene(scene);
  }

  bool Enter(const gltfjson::tree::NodePtr& item, std::u8string_view jsonpath);
  void ShowSelected();
  std::optional<CreateGuiFunc> MatchGui(std::u8string_view jsonpath)
  {
    for (auto& f : m_guiFactories) {
      if (f.Match(jsonpath)) {
        return f.Factory;
      }
    }
    // not found
    return {};
  }

  void ShowSelector(float indent);
  void ShowRight();
};
