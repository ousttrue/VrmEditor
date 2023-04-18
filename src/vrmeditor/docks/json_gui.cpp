#include "json_gui.h"
#include "json_gui_accessor.h"
#include "json_gui_images.h"
#include "json_gui_material.h"
#include "json_gui_mesh.h"
#include <charconv>
#include <imgui.h>
#include <imgui_internal.h>
#include <ranges>
#include <string_view>

JsonGui::JsonGui(const std::shared_ptr<libvrm::gltf::Scene>& scene)
  : m_scene(scene)
{
}

bool
JsonGui::Enter(nlohmann::json& item, std::string_view jsonpath)
{
  static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                         ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                         ImGuiTreeNodeFlags_SpanAvailWidth;
  ImGuiTreeNodeFlags node_flags = base_flags;
  auto is_leaf = !item.is_object() && !item.is_array();
  if (is_leaf) {
    node_flags |=
      ImGuiTreeNodeFlags_Leaf |
      ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
  }
  if (jsonpath == m_selected.Str) {
    node_flags |= ImGuiTreeNodeFlags_Selected;
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
  bool node_open =
    ImGui::TreeNodeEx((void*)(intptr_t)&item, node_flags, "%s", label.c_str());

  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    m_selected.Str = jsonpath;
    m_cache = {};
  }

  return node_open && !is_leaf;
}

// https://github.com/ocornut/imgui/issues/319
static bool
Splitter(bool split_vertically,
         float thickness,
         float* size1,
         float* size2,
         float min_size1,
         float min_size2,
         float splitter_long_axis_size = -1.0f)
{
  using namespace ImGui;
  ImGuiContext& g = *GImGui;
  ImGuiWindow* window = g.CurrentWindow;
  ImGuiID id = window->GetID("##Splitter");
  ImRect bb;
  bb.Min = window->DC.CursorPos +
           (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
  bb.Max = bb.Min + CalcItemSize(split_vertically
                                   ? ImVec2(thickness, splitter_long_axis_size)
                                   : ImVec2(splitter_long_axis_size, thickness),
                                 0.0f,
                                 0.0f);
  return SplitterBehavior(bb,
                          id,
                          split_vertically ? ImGuiAxis_X : ImGuiAxis_Y,
                          size1,
                          size2,
                          min_size1,
                          min_size2,
                          0.0f);
}

void
JsonGui::Show(const std::shared_ptr<libvrm::gltf::Scene>& scene, float indent)
{
  auto enter = [this](nlohmann::json& item, std::string_view jsonpath) {
    return Enter(item, jsonpath);
  };
  auto leave = []() { ImGui::TreePop(); };

  // auto size = ImGui::GetCurrentWindow()->Size;
  auto size = ImGui::GetContentRegionAvail();
  float s = size.y - m_f - 5;
  // ImGui::Text("%f, %f: %f; %f", size.x, size.y, f, s);
  ::Splitter(false, 5, &m_f, &s, 8, 8);
  if (ImGui::BeginChild("##split-first", { size.x, m_f })) {
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, indent);
    scene->TraverseJson(enter, leave);
    ImGui::PopStyleVar();
  }
  ImGui::EndChild();

  if (ImGui::BeginChild("##split-second", { size.x, s })) {
    ImGui::TextUnformatted(m_selected.Str.c_str());

    if (!m_cache) {
      m_cache = CreateGui();
    }
    m_cache();
  }
  ImGui::EndChild();
}

std::function<void()>
JsonGui::CreateGui()
{
  if (m_selected.Size()) {
    auto top = m_selected[0];
    if (top == "accessors") {
      return ShowSelected_accessors(m_scene, m_selected);
    } else if (top == "meshes") {
      return ShowSelected_meshes(m_scene, m_selected);
    } else if (top == "materials") {
      return ShowSelected_materials();
    } else if (top == "images") {
      return ShowSelected_images();
    }
  }

  // default
  return []() {};
}
