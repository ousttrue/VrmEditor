#pragma once
#include <imgui.h>
#include <vrm/gltfroot.h>

class ImHumanoid
{
  std::vector<const char*> m_items;

public:
  void ShowBody(libvrm::GltfRoot& scene)
  {
    m_items.clear();
    for (auto& node : scene.m_nodes) {
      m_items.push_back(node->Name.c_str());
    }

    // bone name, combo box
    if (ImGui::BeginTable(
          "ShowBody", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
      ImGui::TableSetupColumn("Bone", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("Node", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableHeadersRow();
      for (int i = 0; i < (int)libvrm::HumanBones::leftThumbMetacarpal;
           ++i) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", libvrm::HumanBonesNames[i]);

        uint32_t index = -1;
        const char* combo_preview_value = "--";
        // if (auto node_index = scene.m_humanoid[i]) {
        //   index = *node_index;
        //   combo_preview_value = m_items[index];
        // }

        char key[64];
        snprintf(key, sizeof(key), "##%d", i);
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo(key, combo_preview_value, 0)) {
          for (int n = 0; n < m_items.size(); n++) {
            bool is_selected = n == index;
            if (ImGui::Selectable(m_items[n], is_selected)) {
              // TODO: update humanoid
            }

            // Set the initial focus when opening the combo (scrolling +
            // keyboard navigation focus)
            if (is_selected)
              ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }
      }
      ImGui::EndTable();
    }
  }

  void ShowFingers(libvrm::GltfRoot& scene)
  {
    //   ss.str("");
    //   if (auto node_index = humanoid[i]) {
    //     ss << *node_index;
    //   } else {
    //     ss << "--";
    //   }
    //   ImGui::LabelText(vrm::HumanBonesNames[i], "%s", ss.str().c_str());
    // }
    m_items.clear();
    for (auto& node : scene.m_nodes) {
      m_items.push_back(node->Name.c_str());
    }

    // bone name, combo box
    if (ImGui::BeginTable(
          "ShowFingers", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
      ImGui::TableSetupColumn("Bone", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("Left", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("Right", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableHeadersRow();
      for (int i = (int)libvrm::HumanBones::leftThumbMetacarpal;
           i < (int)libvrm::HumanBones::rightThumbMetacarpal;
           ++i) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        // skip "Left" 4
        ImGui::Text("%s", libvrm::HumanBonesNames[i] + 4);

        uint32_t left_index = -1;
        const char* left_combo_preview_value = "--";
        // if (auto node_index = scene.m_humanoid[i]) {
        //   left_index = *node_index;
        //   left_combo_preview_value = m_items[left_index];
        // }

        char left_key[64];
        snprintf(left_key, sizeof(left_key), "##left%d", i);
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo(left_key, left_combo_preview_value, 0)) {
          for (int n = 0; n < m_items.size(); n++) {
            bool is_selected = n == left_index;
            if (ImGui::Selectable(m_items[n], is_selected)) {
              // TODO: update humanoid
            }

            // Set the initial focus when opening the combo (scrolling +
            // keyboard navigation focus)
            if (is_selected)
              ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }

        uint32_t right_index = -1;
        const char* right_combo_preview_value = "--";
        // if (auto node_index = scene.m_humanoid[i + 15]) {
        //   right_index = *node_index;
        //   right_combo_preview_value = m_items[right_index];
        // }
        char right_key[64];
        snprintf(right_key, sizeof(right_key), "##right%d", i + 15);
        ImGui::TableSetColumnIndex(2);
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo(right_key, right_combo_preview_value, 0)) {
          for (int n = 0; n < m_items.size(); n++) {
            bool is_selected = n == right_index;
            if (ImGui::Selectable(m_items[n], is_selected)) {
              // TODO: update humanoid
            }

            // Set the initial focus when opening the combo (scrolling +
            // keyboard navigation focus)
            if (is_selected)
              ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }
      }
      ImGui::EndTable();
    }
  }
};
