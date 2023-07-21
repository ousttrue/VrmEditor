#include "springbone_gui.h"
#include <grapho/imgui/printfbuffer.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <json_widgets.h>
#include <vrm/runtime_node.h>

namespace gui {

static const char*
ToLabel(const std::shared_ptr<libvrm::RuntimeNode>& node)
{
  if (!node) {
    return "--";
  }
  return node->Base->Name.c_str();
}

struct SpringBoneGuiImpl
{
  grapho::imgui::PrintfBuffer buf;
  int m_vrm_version = 0;
  std::shared_ptr<libvrm::RuntimeScene> m_runtime;

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

    std::array<const char*, 3> cols = { "index", "RootNode", "✅" };
    if (grapho::imgui::BeginTableColumns("##_springs", cols)) {
      int i = 0;
      for (auto& spring : m_runtime->m_springBones) {
        ImGui::PushID(spring.get());
        ImGui::TableNextRow();

        // 0
        ImGui::TableNextColumn();
        if (ImGui::Selectable(buf.Printf("%d", i),
                              spring == m_runtime->m_springBoneSelected)) {
          m_runtime->m_springBoneSelected = spring;
        }

        // 1
        ImGui::TableNextColumn();
        std::shared_ptr<libvrm::RuntimeNode> node =
          spring->Joints.empty() ? nullptr : spring->Joints[0]->Head;
        if (m_vrm_version == 0) {
          // VRM-0.X
          if (auto selected = grapho::imgui::SelectVector<
                std::shared_ptr<libvrm::RuntimeNode>>(
                m_runtime->m_nodes, node, &ToLabel)) {
            // joint->Head = *selected;
          }
        } else {
          ImGui::Text("%s", node ? node->Base->Name.c_str() : "--");
        }

        // 2
        ImGui::TableNextColumn();
        if (ImGui::Button("-")) {
          // TODO: remove
        }

        ImGui::PopID();
        ++i;
      }

      // TODO:
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(2);
      if (ImGui::Button("+")) {
      }

      ImGui::EndTable();
    }
  }

  void _EditSpring()
  {
    auto spring = m_runtime->m_springBoneSelected;
    if (!spring) {
      return;
    }

    std::array<const char*, 6> cols = {
      "index", "head", "dragforce", "stiffness", "radius", "✅",
    };
    if (grapho::imgui::BeginTableColumns("##_springs", cols)) {
      int j = 0;
      for (auto joint : spring->Joints) {

        bool editable = true;
        if (m_vrm_version == 0 && j > 0) {
          // VRM-0.x use same setting
          editable = false;
        }

        _EditSpringJoint(j,
                         joint,
                         (j == spring->Joints.size() - 1) && m_vrm_version == 1,
                         editable);

        ++j;
      }

      if (m_vrm_version == 0 && spring->Joints.size() > 0) {
        ImGui::TableNextRow();
        // 0
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("-1");

        // 1
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("tail offset");
      }

      // TODO: add spring
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(5);
      if (ImGui::Button("+")) {
      }

      ImGui::EndTable();
    }
  }

  void _EditSpringJoint(int j,
                        const std::shared_ptr<libvrm::SpringJoint>& joint,
                        bool notUsed,
                        bool editable)
  {
    ImGui::PushID(joint.get());
    ImGui::TableNextRow();

    // 0
    ImGui::TableNextColumn();
    // ImGui::Text("%d", j);
    if (ImGui::Selectable(buf.Printf("%d", j),
                          joint == m_runtime->m_springJointSelected)) {
      m_runtime->SelectJoint(joint);
    }

    // 1
    ImGui::BeginDisabled(!editable);

    ImGui::TableNextColumn();
    if (auto selected =
          grapho::imgui::SelectVector<std::shared_ptr<libvrm::RuntimeNode>>(
            m_runtime->m_nodes, joint->Head, &ToLabel)) {
      joint->Head = *selected;
    }

    if (notUsed) {
      ImGui::TableNextColumn();
      ImGui::TextUnformatted("tail not used");
    } else {

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
    }

    // 5
    ImGui::TableSetColumnIndex(5);
    if (m_vrm_version == 1 && ImGui::Button("-")) {
      // TODO: remove
    }

    ImGui::EndDisabled();
    ImGui::PopID();
  }

  void ShowColliders() {}

  void Show()
  {
    ImGui::RadioButton("vrm-0.x", &m_vrm_version, 0);
    ImGui::SameLine();
    ImGui::RadioButton("vrm-1.0", &m_vrm_version, 1);

    if (ImGui::BeginTabBar("Spring")) {
      if (ImGui::BeginTabItem("Joints")) {
        ShowJoints();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Colliders")) {
        ShowColliders();
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
  }
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
  m_impl->Show();
}

} // namespace
