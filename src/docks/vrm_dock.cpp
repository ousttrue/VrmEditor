#include "vrm_dock.h"
#include <imgui.h>
#include <vrm/vrm0.h>
#include <vrm/vrm1.h>

float
VerticalSlider(int i,
               const char* name,
               float* value,
               const ImVec4& bg,
               const ImVec4 bgHovered,
               const ImVec4& active,
               const ImVec4& grab)
{
  if (i > 0)
    ImGui::SameLine();
  auto x = ImGui::GetCursorScreenPos().x;
  ImGui::PushID(i);
  ImGui::PushStyleColor(ImGuiCol_FrameBg, bg);
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, bgHovered);
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, active);
  ImGui::PushStyleColor(ImGuiCol_SliderGrab, grab);
  ImGui::VSliderFloat("##v", ImVec2(18, 90), value, 0.0f, 1.0f, "");
  if (ImGui::IsItemActive() || ImGui::IsItemHovered())
    ImGui::SetTooltip("%s %.3f", name, *value);
  ImGui::PopStyleColor(4);
  ImGui::PopID();
  return x;
}

void
VrmDock::Create(const AddDockFunc& addDock,
                std::string_view title,
                const std::shared_ptr<gltf::Scene>& scene)
{
  addDock(Dock(title, [scene]() {
    if (auto vrm = scene->m_vrm0) {
      ImGui::Text("%s", "vrm-0.x");

      const float spacing = 4;
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, spacing));
      ImGui::PushID("set1");
      // for (auto expression : vrm->m_expressions) {
      //   ImGui::SliderFloat(
      //     expression->label.c_str(), &expression->weight, 0, 1);
      // }
      {
        float dummy = 0;
        // 喜
        auto happy = vrm->m_expressions.Get(vrm::v0::ExpressionPreset::joy);
        auto x0 = VerticalSlider(0,
                                 "joy",
                                 happy ? &happy->weight : &dummy,
                                 (ImVec4)ImColor::HSV(0 / 7.0f, 0.5f, 0.5f),
                                 (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.5f),
                                 (ImVec4)ImColor::HSV(0 / 7.0f, 0.7f, 0.5f),
                                 (ImVec4)ImColor::HSV(0 / 7.0f, 0.9f, 0.9f));
        // 怒
        auto angry = vrm->m_expressions.Get(vrm::v0::ExpressionPreset::angry);
        auto x1 = VerticalSlider(1,
                                 "angry",
                                 angry ? &angry->weight : &dummy,
                                 (ImVec4)ImColor::HSV(0 / 7.0f, 0.5f, 0.5f),
                                 (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.5f),
                                 (ImVec4)ImColor::HSV(0 / 7.0f, 0.7f, 0.5f),
                                 (ImVec4)ImColor::HSV(0 / 7.0f, 0.9f, 0.9f));
        // 哀
        auto sad = vrm->m_expressions.Get(vrm::v0::ExpressionPreset::sorrow);
        auto x2 = VerticalSlider(2,
                                 "sorrow",
                                 sad ? &sad->weight : &dummy,
                                 (ImVec4)ImColor::HSV(0 / 7.0f, 0.5f, 0.5f),
                                 (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.5f),
                                 (ImVec4)ImColor::HSV(0 / 7.0f, 0.7f, 0.5f),
                                 (ImVec4)ImColor::HSV(0 / 7.0f, 0.9f, 0.9f));
        // 楽
        auto relaxed = vrm->m_expressions.Get(vrm::v0::ExpressionPreset::fun);
        auto x3 = VerticalSlider(3,
                                 "fun",
                                 relaxed ? &relaxed->weight : &dummy,
                                 (ImVec4)ImColor::HSV(0 / 7.0f, 0.5f, 0.5f),
                                 (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.5f),
                                 (ImVec4)ImColor::HSV(0 / 7.0f, 0.7f, 0.5f),
                                 (ImVec4)ImColor::HSV(0 / 7.0f, 0.9f, 0.9f));

        // static int int_value = 0;
        // ImGui::VSliderInt("##int", ImVec2(18, 160), &int_value, 0, 5);
        // ImGui::SameLine();
        // static float values[7] = {
        //   0.0f, 0.60f, 0.35f, 0.9f, 0.70f, 0.20f, 0.0f
        // };
        // for (int i = 0; i < 7; i++)

        // ImGui::SameLine();
        // ImGui::PushID("set2");
        // static float values2[4] = { 0.20f, 0.80f, 0.40f, 0.25f };
        // const int rows = 3;
        // const ImVec2 small_slider_size(
        //   18, (float)(int)((160.0f - (rows - 1) * spacing) / rows));
        // for (int nx = 0; nx < 4; nx++) {
        //   if (nx > 0)
        //     ImGui::SameLine();
        //   ImGui::BeginGroup();
        //   for (int ny = 0; ny < rows; ny++) {
        //     ImGui::PushID(nx * rows + ny);
        //     ImGui::VSliderFloat(
        //       "##v", small_slider_size, &values2[nx], 0.0f, 1.0f, "");
        //     if (ImGui::IsItemActive() || ImGui::IsItemHovered())
        //       ImGui::SetTooltip("%.3f", values2[nx]);
        //     ImGui::PopID();
        //   }
        //   ImGui::EndGroup();
        // }
        // ImGui::PopID();
        //
        // ImGui::SameLine();
        // ImGui::PushID("set3");
        // for (int i = 0; i < 4; i++) {
        //   if (i > 0)
        //     ImGui::SameLine();
        //   ImGui::PushID(i);
        //   ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
        //   ImGui::VSliderFloat(
        //     "##v", ImVec2(40, 160), &values[i], 0.0f, 1.0f, "%.2f\nsec");
        //   ImGui::PopStyleVar();
        //   ImGui::PopID();
        // }
        ImGui::PopID();
        ImGui::PopStyleVar();
        // ImGui::TreePop();

        auto drawList = ImGui::GetWindowDrawList();
        auto cursor = ImGui::GetCursorScreenPos();
        drawList->AddText({ x0, cursor.y }, IM_COL32_BLACK, "󰱰 ", nullptr);
        drawList->AddText({ x1, cursor.y }, IM_COL32_BLACK, "󰱩 ", nullptr);
        drawList->AddText({ x2, cursor.y }, IM_COL32_BLACK, "󰱶 ", nullptr);
        drawList->AddText({ x3, cursor.y }, IM_COL32_BLACK, "󰱱 ", nullptr);
        auto textSize = ImGui::CalcTextSize(" ");
        auto lineheight = textSize.y;
        ImGui::SetCursorPosY(cursor.y + lineheight);
      }
    }
    if (auto vrm = scene->m_vrm1) {
      ImGui::Text("%s", "vrm-1.0");
      // for (auto expression : vrm->m_expressions) {
      //   ImGui::SliderFloat(
      //     expression->label.c_str(), &expression->weight, 0, 1);
      // }
    }
  }));
}
