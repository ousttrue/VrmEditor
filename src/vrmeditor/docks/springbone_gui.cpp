#include "springbone_gui.h"
#include <grapho/imgui/printfbuffer.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <json_widgets.h>
#include <vrm/runtime_node.h>

namespace gui {

struct SpringBoneGuiImpl
{
  std::shared_ptr<libvrm::RuntimeScene> m_runtime;
  // current selected
  std::shared_ptr<libvrm::SpringBone> m_spring;

  void SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime)
  {
    m_runtime = runtime;
  }

  void ShowJoints()
  {
    auto size = ImGui::GetContentRegionAvail();
    static float sz1 = 300;
    float sz2 = size.x - sz1 - 5;
    grapho::imgui::Splitter(true, 5.0f, &sz1, &sz2, 100, 100);
    {
      ImGui::BeginChild("1", ImVec2(sz1, -1));
      _SelectSpring();
      ImGui::EndChild();
    }
    {
      ImGui::SameLine();
      ImGui::BeginChild("2", ImVec2(sz2, -1));
      _EditSpring();
      ImGui::EndChild();
    }
  }

  void _SelectSpring()
  {
    if (!m_runtime) {
      return;
    }

    grapho::imgui::PrintfBuffer buf;
    std::array<const char*, 2> cols = { "index", "RootNode" };
    if (grapho::imgui::BeginTableColumns("##_springs", cols)) {
      int i = 0;
      for (auto& spring : m_runtime->m_springBones) {
        ImGui::TableNextRow();

        // 0
        ImGui::TableNextColumn();
        if (ImGui::Selectable(buf.Printf("%d", i), spring == m_spring)) {
          m_spring = spring;
        }

        // 1
        ImGui::TableNextColumn();

        ++i;
      }
      ImGui::EndTable();
    }
  }

  std::shared_ptr<libvrm::RuntimeNode> SelectNode(
    const std::vector<std::shared_ptr<libvrm::RuntimeNode>>& nodes,
    const std::shared_ptr<libvrm::RuntimeNode>& current)
  {
    std::shared_ptr<libvrm::RuntimeNode> selected;
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##_SelectNode",
                          current ? current->Base->Name.c_str() : "--")) {
      for (auto& node : nodes) {
        bool is_selected = (current == node);
        if (ImGui::Selectable(node->Base->Name.c_str(), is_selected)) {
          selected = node;
        }
        if (is_selected) {
          // Set the initial focus when opening the combo (scrolling + for
          // keyboard navigation support in the upcoming navigation branch)
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    return selected;
  }

  void _EditSpring()
  {
    if (!m_spring) {
      return;
    }

    std::array<const char*, 5> cols = {
      "index", "head", "dragforce", "stiffness", "radius",
    };
    if (grapho::imgui::BeginTableColumns("##_springs", cols)) {
      int j = 0;
      for (auto joint : m_spring->Joints) {
        ImGui::PushID(joint.get());
        ImGui::TableNextRow();

        // 0
        ImGui::TableNextColumn();
        ImGui::Text("%d", j);

        // 1
        ImGui::TableNextColumn();
        if (auto selected = SelectNode(m_runtime->m_nodes, joint->Head)) {
          joint->Head = selected;
        }
        ImGui::SameLine();
        ImGui::Text("%s", joint->Head->Base->Name.c_str());

        // 2
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputFloat("##_DragForce", &joint->DragForce);

        // 3
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputFloat("##_Stiffness", &joint->Stiffness);

        // 4
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputFloat("##_Radius", &joint->Radius);

        ImGui::PopID();
        ++j;
      }
      ImGui::EndTable();
    }
  }

  void ShowColliders() {}
};

//
// SpringBoneGui
//
SpringBoneGui::SpringBoneGui()
  : m_impl(new SpringBoneGuiImpl)
{
}

SpringBoneGui::~SpringBoneGui()
{
  delete m_impl;
}

void
SpringBoneGui::SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime)
{
  m_impl->SetRuntime(runtime);
}

void
SpringBoneGui::ShowGui()
{
  if (ImGui::BeginTabBar("Spring")) {
    if (ImGui::BeginTabItem("Joints")) {
      m_impl->ShowJoints();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Colliders")) {
      m_impl->ShowColliders();
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
}

} // namespace
