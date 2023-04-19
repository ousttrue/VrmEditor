#pragma once
#include <sstream>
#include <vector>
#include <vrm/jsonpath.h>
#include <vrm/scene.h>

using ShowGui = std::function<void()>;

using ShowGuiFactory =
  std::function<ShowGui(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                        std::string_view jsonpath)>;

struct JsonGuiFactory
{
  std::string m_match;
  ShowGuiFactory Factory;

  bool Match(std::string_view jsonpath)
  {
    return libvrm::JsonPath(m_match).Match(jsonpath);
  }
};

struct JsonGui
{
  std::stringstream m_ss;
  float m_f = 500;

  std::list<JsonGuiFactory> m_factories;

  std::shared_ptr<libvrm::gltf::Scene> m_scene;
  std::string m_selected;
  ShowGui m_cache = []() {};

  JsonGui(const std::shared_ptr<libvrm::gltf::Scene>& scene);
  bool Enter(nlohmann::json& item, std::string_view jsonpath);
  void Show(float indent);
  void ShowSelected();
  std::optional<ShowGuiFactory> Match(std::string_view jsonpath);
};
