#include "json_dock.h"
#include <imgui.h>
#include <sstream>

struct JsonDockImpl
{
  std::stringstream m_ss;

  bool Enter(nlohmann::json& item, std::span<std::string_view> jsonpath)
  {
    static ImGuiTreeNodeFlags base_flags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
      ImGuiTreeNodeFlags_SpanAvailWidth;
    ImGuiTreeNodeFlags node_flags = base_flags;
    auto is_leaf = !item.is_object() && !item.is_array();
    if (is_leaf) {
      node_flags |=
        ImGuiTreeNodeFlags_Leaf |
        ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
    }

    auto key = jsonpath.back();
    m_ss.str("");
    if (item.is_object()) {
      if (item.find("name") != item.end()) {
        m_ss << key << ": " << (std::string_view)item.at("name");
      } else {
        m_ss << key << ": object";
      }
    } else if (item.is_array()) {
      m_ss << key << ": [" << item.size() << "]";
    } else {
      m_ss << key << ": " << item.dump();
    }
    auto label = m_ss.str();
    bool node_open = ImGui::TreeNodeEx(
      (void*)(intptr_t)&item, node_flags, "%s", label.c_str());
    return node_open && !is_leaf;
  }
};

void
JsonDock::Create(const AddDockFunc& addDock,
                 std::string_view title,
                 const std::shared_ptr<gltf::Scene>& scene,
                 float indent)
{
  auto impl = std::make_shared<JsonDockImpl>();

  auto enter = [impl](nlohmann::json& item,
                      std::span<std::string_view> jsonpath) {
    return impl->Enter(item, jsonpath);
  };
  auto leave = []() { ImGui::TreePop(); };

  addDock(Dock(title, [scene, enter, leave, indent]() {
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, indent);
    scene->TraverseJson(enter, leave);
    ImGui::PopStyleVar();
  }));
}
