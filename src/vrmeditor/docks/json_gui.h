#pragma once
#include "gui.h"
#include <imgui.h>
#include <sstream>
#include <vector>
#include <vrm/jsonpath.h>
#include <vrm/scene.h>

using ShowGui = std::function<void()>;

using CreateGuiFunc =
  std::function<ShowGui(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                        std::string_view jsonpath)>;

struct JsonGuiFactory
{
  std::string m_match;
  CreateGuiFunc Factory;

  bool Match(std::string_view jsonpath)
  {
    return libvrm::JsonPath(m_match).Match(jsonpath);
  }
};

using CreateLabelFunc =
  std::function<std::string(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                            std::string_view jsonpath)>;

struct JsonLabelFactory
{
  std::string m_match;
  CreateLabelFunc Factory;

  bool Match(std::string_view jsonpath)
  {
    return libvrm::JsonPath(m_match).Match(jsonpath);
  }
};

struct JsonGui
{
  // splitter ?
  float m_f = 300;

  std::list<JsonGuiFactory> m_guiFactories;
  std::list<JsonLabelFactory> m_labelFactories;

  std::shared_ptr<libvrm::gltf::Scene> m_scene;
  std::string m_selected;
  ShowGui m_cache;
  std::unordered_map<std::string, std::string> m_labelCache;

  JsonGui();

  void SetScene(const std::shared_ptr<libvrm::gltf::Scene>& scene)
  {
    m_scene = scene;
    m_cache = {};
    m_labelCache.clear();
  }

  // bool Enter(nlohmann::json& item, std::string_view jsonpath);
  void Show(float indent);
  void ShowSelected();
  std::optional<CreateGuiFunc> MatchGui(std::string_view jsonpath)
  {
    for (auto& f : m_guiFactories) {
      if (f.Match(jsonpath)) {
        return f.Factory;
      }
    }
    // not found
    return {};
  }
  std::optional<CreateLabelFunc> MatchLabel(std::string_view jsonpath)
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
