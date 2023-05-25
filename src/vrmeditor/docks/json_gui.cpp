#include "json_gui.h"
#include "json_gui_factory.h"
#include "json_gui_labelcache.h"
// #include "json_gui_accessor.h"
// #include "json_gui_images.h"
// #include "json_gui_material.h"
// #include "json_gui_mesh.h"
// #include "json_gui_node.h"
// #include "json_gui_skin.h"
// #include "json_gui_vrm0.h"
// #include "json_gui_vrm1.h"
#include <charconv>
#include <imgui.h>
#include <imgui_internal.h>
#include <ranges>
#include <sstream>
#include <string_view>

JsonGui::JsonGui()
  : m_label(new LabelCacheManager)
  , m_inspector(new JsonGuiFactoryManager)
{
}

void
JsonGui::SetScene(const gltfjson::tree::NodePtr& gltf)
{
  m_gltf = gltf;
  m_label->Clear();
  m_inspector->Clear();
}

bool
JsonGui::Enter(const gltfjson::tree::NodePtr& item, std::u8string_view jsonpath)
{
  static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                         ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                         ImGuiTreeNodeFlags_SpanAvailWidth;
  ImGuiTreeNodeFlags node_flags = base_flags;
  auto is_leaf = !item->Object() && !item->Array();
  if (is_leaf) {
    node_flags |=
      ImGuiTreeNodeFlags_Leaf |
      ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
  }
  if (m_inspector->IsSelected(jsonpath)) {
    node_flags |= ImGuiTreeNodeFlags_Selected;
  }

  auto& label = m_label->Get(item, jsonpath);

  ImGui::TableNextRow();

  // 0
  ImGui::TableNextColumn();
  bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)item.get(),
                                     node_flags,
                                     "%s",
                                     (const char*)label.Key.c_str());

  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    m_inspector->Select(jsonpath);
  }

  // 1
  ImGui::TableNextColumn();
  ImGui::TextUnformatted((const char*)label.Value.c_str());

  return node_open && !is_leaf;
}

void
JsonGui::ShowSelector(float indent)
{
  auto enter = [this](const gltfjson::tree::NodePtr& item,
                      std::u8string_view jsonpath) {
    return Enter(item, jsonpath);
  };
  auto leave = []() { ImGui::TreePop(); };

  static ImGuiTableFlags flags =
    ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH |
    ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
    ImGuiTableFlags_NoBordersInBody;

  if (ImGui::BeginTable("3ways", 2, flags)) {
    // The first column will use the default _WidthStretch when ScrollX is Off
    // and _WidthFixed when ScrollX is On
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
    ImGui::TableSetupColumn(
      "Value", ImGuiTableColumnFlags_WidthFixed, 20 * 12.0f);
    ImGui::TableHeadersRow();

    {
      // tree
      ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, indent);
      gltfjson::tree::TraverseJson(enter, leave, m_gltf);
      ImGui::PopStyleVar();
    }

    ImGui::EndTable();
  }
}

void
JsonGui::ShowSelected()
{
  m_inspector->ShowGui(m_gltf);
}
