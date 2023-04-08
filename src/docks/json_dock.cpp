#include "json_dock.h"
#include <imgui.h>
#include <ranges>
#include <sstream>

struct JsonDockImpl
{
  std::stringstream m_ss;

  bool Enter(nlohmann::json& item, std::string_view jsonpath)
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

    m_ss.str("");

    bool extension = false;
    std::string_view kind;
    std::string_view key;
    int i = 0;
    for (auto jp : jsonpath | std::views::split('.')) {
      key = std::string_view(jp);
      if (i == 0) {
        kind = key;
      }
      ++i;
      if (key == "extensions" || key == "extrans") {
        extension = true;
        break;
      }
    }

    if (extension) {
      m_ss << " ";
    } else if (kind == "images" || kind == "textures" || kind == "samplers") {
      m_ss << " ";
    } else if (kind == "bufferViews" || kind == "buffers") {
      m_ss << " ";
    } else if (kind == "accessors") {
      m_ss << " ";
    } else if (kind == "meshes" || kind == "skins") {
      m_ss << "󰕣 ";
    } else if (kind == "materials") {
      m_ss << " ";
    } else if (kind == "nodes" || kind == "scenes" || kind == "scene") {
      m_ss << "󰵉 ";
    } else if (kind == "animations") {
      m_ss << " ";
    } else if (kind == "asset") {
      m_ss << " ";
    } else if (kind == "extensionsUsed") {
      m_ss << " ";
    }

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

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
      // context->new_selected = node;
    }

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

  auto enter = [impl](nlohmann::json& item, std::string_view jsonpath) {
    return impl->Enter(item, jsonpath);
  };
  auto leave = []() { ImGui::TreePop(); };

  addDock(Dock(title, [scene, enter, leave, indent]() {
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, indent);
    scene->TraverseJson(enter, leave);
    ImGui::PopStyleVar();
  }));
}
