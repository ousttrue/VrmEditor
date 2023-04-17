#pragma once
#include <sstream>
#include <vector>
#include <vrm/scene.h>

struct JsonDockImpl
{
  std::stringstream m_ss;
  float m_f = 500;

  std::string m_selected;
  std::vector<std::string_view> m_jsonpath;
  std::shared_ptr<libvrm::gltf::Scene> m_scene;

  JsonDockImpl(const std::shared_ptr<libvrm::gltf::Scene>& scene);
  std::optional<int> GetIndex(int n)const;
  void SetSelected(std::string_view selected);
  bool Enter(nlohmann::json& item, std::string_view jsonpath);
  void Show(const std::shared_ptr<libvrm::gltf::Scene>& scene, float indent);
  void ShowSelected();
  void ShowSelected_accessors();
  void ShowSelected_images();
  void ShowSelected_materials();
  void ShowSelected_meshes();
  void ShowSelected_prims();
};
