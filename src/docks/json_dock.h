#pragma once
#include "gui.h"
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string_view>
#include <vrm/scene.h>

class JsonDock
{
public:
  static std::stringstream s_ss;
  static void Create(const AddDockFunc& addDock,
                     std::string_view title,
                     const std::shared_ptr<gltf::Scene>& scene,
                     float indent)
  {

    auto enter = [](nlohmann::json& item, const std::string& key) {
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

      s_ss.str("");
      if (item.is_object()) {
        if (item.find("name") != item.end()) {
          s_ss << key << ": " << (std::string_view)item.at("name");
        } else {
          s_ss << key << ": object";
        }
      } else if (item.is_array()) {
        s_ss << key << ": [" << item.size() << "]";
      } else {
        s_ss << key << ": " << item.dump();
      }
      auto label = s_ss.str();
      bool node_open = ImGui::TreeNodeEx(
        (void*)(intptr_t)&item, node_flags, "%s", label.c_str());
      return node_open && !is_leaf;
    };
    auto leave = []() { ImGui::TreePop(); };

    addDock(Dock(title, [scene, enter, leave, indent]() {
      ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, indent);
      scene->TraverseJson(enter, leave);
      ImGui::PopStyleVar();
    }));
  }
};
