#pragma once
#include "gui.h"
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

using CreateLabelFunc = std::function<std::u8string(
  const std::shared_ptr<libvrm::gltf::GltfRoot>& scene,
  std::u8string_view jsonpath)>;

struct JsonLabelFactory
{
  std::u8string m_match;
  CreateLabelFunc Factory;

  bool Match(std::u8string_view jsonpath)
  {
    return gltfjson::JsonPath(m_match).Match(jsonpath);
  }
};

struct JsonGui
{
  // splitter ?
  float m_f = 300;

  std::list<JsonGuiFactory> m_guiFactories;
  std::list<JsonLabelFactory> m_labelFactories;

  std::shared_ptr<libvrm::gltf::GltfRoot> m_scene;
  std::u8string m_selected;
  ShowGui m_cache;
  std::unordered_map<std::u8string, std::u8string> m_labelCache;

  JsonGui();

  void SetScene(const std::shared_ptr<libvrm::gltf::GltfRoot>& scene)
  {
    m_scene = scene;
    m_cache = {};
    m_labelCache.clear();
  }

  bool Enter(const gltfjson::tree::NodePtr& item, std::u8string_view jsonpath);
  void Show(float indent);
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

  void Show(const char* title, bool* p_open, float indent);
};
