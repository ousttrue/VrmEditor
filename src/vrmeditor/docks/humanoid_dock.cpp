#include "humanoid_dock.h"
#include "app.h"
#include "humanpose/humanpose_stream.h"
#include "platform.h"
#include <grapho/imgui/printfbuffer.h>
#include <imgui.h>
#include <vrm/gltfroot.h>
#include <vrm/humanoid/humanbones.h>
#include <vrm/node.h>

struct ImHumanoid
{
  std::shared_ptr<libvrm::RuntimeScene> m_runtime;

  void SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime)
  {
    m_runtime = runtime;
  }

  void BoneNodeSelector(libvrm::HumanBones bone)
  {
    if (!m_runtime) {
      return;
    }
    auto scene = m_runtime->m_base;

    uint32_t index = -1;
    const char* combo_preview_value = "--";
    auto [node, node_index] = scene->GetBoneNode(bone);
    if (node) {
      index = node_index;
      combo_preview_value = node->Name.c_str();
    }

    char key[64];
    snprintf(key, sizeof(key), "##humanbone%d", (int)bone);
    if (ImGui::BeginCombo(key, combo_preview_value, 0)) {
      for (int n = 0; n < scene->m_nodes.size(); n++) {
        bool is_selected = n == index;
        auto& node = scene->m_nodes[n];
        if (ImGui::Selectable(node->Name.c_str(), is_selected)) {
          for (auto& node : scene->m_nodes) {
            if (node->Humanoid == bone) {
              // clear old bone
              node->Humanoid = std::nullopt;
            }
          }
          node->Humanoid = bone;
        }

        // Set the initial focus when opening the combo (scrolling +
        // keyboard navigation focus)
        if (is_selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
  }

  void ShowBody()
  {
    // bone name, combo box
    if (ImGui::BeginTable(
          "ShowBody", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
      ImGui::TableSetupColumn("Bone", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("Node", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableHeadersRow();
      for (auto bone : libvrm::HumanBonesRange::Body) {
        ImGui::TableNextRow();

        // 0
        ImGui::TableNextColumn();
        ImGui::Text("%s", libvrm::HumanBonesNamesWithIcon[(int)bone]);

        // 1
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        BoneNodeSelector(bone);
      }
      ImGui::EndTable();
    }
  }

  void ShowFingers()
  {
    // bone name, combo box
    if (ImGui::BeginTable(
          "ShowFingers", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
      ImGui::TableSetupColumn("Bone", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("Left", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("Right", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableHeadersRow();
      for (auto bone : libvrm::HumanBonesRange::LeftFingers) {
        ImGui::TableNextRow();

        // 0
        ImGui::TableNextColumn();
        // skip "Left" 4
        ImGui::Text("🖐%s", libvrm::HumanBonesNames[(int)bone] + 4);

        // 1
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        BoneNodeSelector(bone);

        // 2
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        BoneNodeSelector((libvrm::HumanBones)((int)bone + 15));
      }
      ImGui::EndTable();
    }
  }

  // std::span<const HumanBones> Bones;
  // std::span<const DirectX::XMFLOAT4> Rotations;
  void ShowPose()
  {
    if (!m_runtime) {
      return;
    }
    auto pose = m_runtime->CurrentHumanPose();
    ImGui::BeginDisabled(true);
    ImGui::InputFloat3("Root Position", &pose.RootPosition.x);
    for (int i = 0; i < pose.Bones.size(); ++i) {
      auto label = libvrm::HumanBoneToNameWithIcon(pose.Bones[i]);
      ImGui::InputFloat4(label, (float*)&pose.Rotations[i].x);
    }
    ImGui::EndDisabled();
  }
};

HumanoidDock::HumanoidDock()
  : m_humanoid(new ImHumanoid)
{
}

HumanoidDock::~HumanoidDock()
{
  delete m_humanoid;
}

void
HumanoidDock::SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime)
{
  m_humanoid->SetRuntime(runtime);
}

void
HumanoidDock::ShowGui()
{
  if (ImGui::BeginTabBar("HierarchyTabs")) {
    if (ImGui::BeginTabItem("🦴Bones")) {
      ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
      if (ImGui::CollapsingHeader("Body")) {
        m_humanoid->ShowBody();
      }
      if (ImGui::CollapsingHeader("Fingers")) {
        m_humanoid->ShowFingers();
      }
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("🏃Input")) {
      humanpose::HumanPoseStream::Instance().ShowGui();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("🧍Pose")) {
      if (m_humanoid->m_runtime) {
        if (ImGui::Button("Copy")) {
          auto humanpose = m_humanoid->m_runtime->CopyVrmPoseText();
          Platform::Instance().CopyText(humanpose);
        }
        if (ImGui::Button("Paste as scene")) {
          auto json = Platform::Instance().PasteText();
          app::LoadGltfString(json);
        }
        if (ImGui::Button("Paste as pose")) {
          auto json = Platform::Instance().PasteText();
          humanpose::HumanPoseStream::Instance().LoadVrmPose(json);
        }
      }
      m_humanoid->ShowPose();
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
}
