#include "hierarchy_gui.h"
#include "gui.h"
#include <imgui.h>

#include <grapho/imgui/printfbuffer.h>
#include <grapho/imgui/widgets.h>
#include <vrm/humanoid/humanbones.h>
#include <vrm/runtime_node.h>
#include <vrm/runtime_scene.h>

struct HierarchyGuiImpl
{
  std::shared_ptr<libvrm::RuntimeScene> m_scene;
  grapho::imgui::PrintfBuffer m_print;

  void SetScene(const std::shared_ptr<libvrm::RuntimeScene>& scene)
  {
    m_scene = scene;
  }

  void Traverse(const std::shared_ptr<libvrm::RuntimeNode>& node)
  {
    static ImGuiTreeNodeFlags base_flags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
      ImGuiTreeNodeFlags_SpanAvailWidth;
    ImGuiTreeNodeFlags node_flags = base_flags;
    auto is_leaf = node->Children.size() == 0;
    if (is_leaf) {
      node_flags |=
        ImGuiTreeNodeFlags_Leaf |
        ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
    }
    if (node == m_scene->m_selected) {
      node_flags |= ImGuiTreeNodeFlags_Selected;
    }

    // auto& label = m_label->Get(item, jsonpath);

    ImGui::TableNextRow();

    // 0
    ImGui::TableNextColumn();
    // ImGui::SetNextItemOpen(true, ImGuiCond_Once);

    bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)node.get(),
                                       node_flags,
                                       "%s",
                                       (const char*)node->Node->Name.c_str());

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
      m_scene->m_selected = node;
    }

    // 1
    if (auto humanoid = node->Node->Humanoid) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted(libvrm::HumanBoneToName(*humanoid));
    }

    if (node_open) {
      for (auto& child : node->Children) {
        Traverse(child);
      }
      if (!is_leaf) {
        ImGui::TreePop();
      }
    }
  }

  void ShowGui()
  {
    if (!m_scene) {
      return;
    }

    std::array<const char*, 2> cols = { "name", "humanoid" };

    if (grapho::imgui::BeginTableColumns("##JsonGui::ShowSelector", cols)) {
      // tree
      ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing,
                          Gui::Instance().Indent());

      for (auto& root : m_scene->m_roots) {
        Traverse(root);
      }

      ImGui::PopStyleVar();
      ImGui::EndTable();
    }
  }
};

HierarchyGui::HierarchyGui()
  : m_impl(new HierarchyGuiImpl)
{
}

HierarchyGui::~HierarchyGui()
{
  delete m_impl;
}

void
HierarchyGui::SetRuntimeScene(
  const std::shared_ptr<libvrm::RuntimeScene>& scene)
{
  m_impl->SetScene(scene);
}

void
HierarchyGui::ShowGui()
{
  m_impl->ShowGui();
}
