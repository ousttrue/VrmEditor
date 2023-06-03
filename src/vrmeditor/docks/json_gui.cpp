#include "json_gui.h"
#include "json_gui_factory.h"
#include "json_gui_labelcache.h"
#include <charconv>
#include <glr/gl3renderer.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <ranges>
#include <sstream>
#include <string_view>

JsonGui::JsonGui()
  : m_label(new LabelCacheManager)
  , m_inspector(new JsonGuiFactoryManager)
{

  m_inspector->OnUpdated([](auto jsonpath) {
    gltfjson::JsonPath path(jsonpath);
    auto [childOfRoot, i] = path.GetChildOfRootIndex();
    if (childOfRoot == u8"materials") {
      glr::ReleaseMaterial(i);
    }
  });

  m_inspector->OnUpdated(std::bind(
    &LabelCacheManager::ClearCache, m_label.get(), std::placeholders::_1));
}

void
JsonGui::SetScene(const std::shared_ptr<libvrm::gltf::GltfRoot>& root)
{
  m_root = root;
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
  auto is_leaf = item->Size() == 0;
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
  if (m_inspector->ShouldOpen(jsonpath)) {
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  }

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
  if (!m_root) {
    return;
  }

  auto enter = [this](const gltfjson::tree::NodePtr& item,
                      std::u8string_view jsonpath) {
    return Enter(item, jsonpath);
  };
  auto leave = []() { ImGui::TreePop(); };

  std::array<const char*, 2> cols = {
    "Name",
    "Value",
  };

  if (grapho::imgui::BeginTableColumns("##JsonGui::ShowSelector", cols)) {

    // tree
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, indent);
    gltfjson::tree::TraverseJson(enter, leave, m_root->m_gltf->m_json);
    ImGui::PopStyleVar();

    ImGui::EndTable();
  }
}

void
JsonGui::ShowSelected()
{
  if (!m_root) {
    return;
  }
  m_inspector->ShowGui(*m_root->m_gltf, m_root->m_bin);
}
