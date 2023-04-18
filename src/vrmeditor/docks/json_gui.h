#pragma once
#include <sstream>
#include <vector>
#include <vrm/jsonpath.h>
#include <vrm/scene.h>

using ShowGui = std::function<void()>;

struct JsonGui
{
  std::stringstream m_ss;
  float m_f = 500;

  std::shared_ptr<libvrm::gltf::Scene> m_scene;
  libvrm::JsonPath m_selected;
  ShowGui m_cache;

  JsonGui(const std::shared_ptr<libvrm::gltf::Scene>& scene);
  bool Enter(nlohmann::json& item, std::string_view jsonpath);
  void Show(float indent);
  void ShowSelected();

private:
  ShowGui CreateGui();
};
