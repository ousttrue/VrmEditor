#include "vrm_dock.h"
#include <imgui.h>

struct SliderColor
{
  ImVec4 bg;
  ImVec4 bgHovered;
  ImVec4 active;
  ImVec4 grab;

  static SliderColor FromHue(float hue)
  {
    return {
      ImColor::HSV(hue, 0.5f, 0.5f),
      ImColor::HSV(hue, 0.6f, 0.5f),
      ImColor::HSV(hue, 0.7f, 0.5f),
      ImColor::HSV(hue, 0.9f, 0.9f),
    };
  }
};

float
VSlider(int i, const char* name, float* value, const SliderColor& colors)
{
  if (i > 0)
    ImGui::SameLine();
  auto x = ImGui::GetCursorScreenPos().x;
  ImGui::PushID(i);
  ImGui::PushStyleColor(ImGuiCol_FrameBg, colors.bg);
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, colors.bgHovered);
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, colors.active);
  ImGui::PushStyleColor(ImGuiCol_SliderGrab, colors.grab);

  bool button_disable = false;
  float dummy = 0;
  if (!value) {
    value = &dummy;
    button_disable = true;
  }
  ImGui::BeginDisabled(button_disable);
  ImGui::VSliderFloat("##v", ImVec2(24, 90), value, 0.0f, 1.0f, "");
  if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
    ImGui::SetTooltip("%s %.3f", name, *value);
  }
  ImGui::EndDisabled();
  ImGui::PopStyleColor(4);
  ImGui::PopID();
  return x;
}

struct SliderLabel
{
  float X;
  const char* Label;
};

static bool
Enable(const std::shared_ptr<libvrm::Expression>& ex)
{
  if (!ex)
    return false;
  return !ex->Empty();
}

class VrmGui
{
  std::shared_ptr<libvrm::RuntimeScene> m_scene;

public:
  VrmGui(const std::shared_ptr<libvrm::RuntimeScene>& scene)
    : m_scene(scene)
  {
  }

  void ShowExpression()
  {
    if (auto ex = m_scene->m_expressions) {
      // ImGui::Text("%s", "expressions");
      const float spacing = 4;
      {
        static std::vector<SliderLabel> s_labels;
        s_labels.clear();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                            ImVec2(spacing, spacing));
        ImGui::PushID("set1");
        // 喜怒哀楽驚
        auto emotion = SliderColor::FromHue(2.0f / 7);
        auto happy = ex->Get(libvrm::ExpressionPreset::happy);
        int i = 0;
        s_labels.push_back({
          VSlider(
            i++, "happy", Enable(happy) ? &happy->weight : nullptr, emotion),
          "喜",
        });
        auto angry = ex->Get(libvrm::ExpressionPreset::angry);
        s_labels.push_back({
          VSlider(
            i++, "angry", Enable(angry) ? &angry->weight : nullptr, emotion),
          "怒",
        });
        auto sad = ex->Get(libvrm::ExpressionPreset::sad);
        s_labels.push_back({
          VSlider(i++, "sad", Enable(sad) ? &sad->weight : nullptr, emotion),
          "哀",
        });
        auto relaxed = ex->Get(libvrm::ExpressionPreset::relaxed);
        s_labels.push_back({
          VSlider(i++,
                  "relaxed",
                  Enable(relaxed) ? &relaxed->weight : nullptr,
                  emotion),
          "楽",
        });
        auto surprised = ex->Get(libvrm::ExpressionPreset::surprised);
        s_labels.push_back({
          VSlider(i++,
                  "surprised",
                  Enable(surprised) ? &surprised->weight : nullptr,
                  emotion),
          "驚",
        });
        // lipsync
        auto lipsync = SliderColor::FromHue(0.0f / 7);
        auto lip_aa = ex->Get(libvrm::ExpressionPreset::aa);
        s_labels.push_back({
          VSlider(
            i++, "aa", Enable(lip_aa) ? &lip_aa->weight : nullptr, lipsync),
          "aa",
        });
        auto lip_ih = ex->Get(libvrm::ExpressionPreset::ih);
        s_labels.push_back({
          VSlider(
            i++, "ih", Enable(lip_ih) ? &lip_ih->weight : nullptr, lipsync),
          "ih",
        });
        auto lip_ou = ex->Get(libvrm::ExpressionPreset::ou);
        s_labels.push_back({
          VSlider(
            i++, "ou", Enable(lip_ou) ? &lip_ou->weight : nullptr, lipsync),
          "ou",
        });
        auto lip_ee = ex->Get(libvrm::ExpressionPreset::ee);
        s_labels.push_back({
          VSlider(
            i++, "ee", Enable(lip_ee) ? &lip_ee->weight : nullptr, lipsync),
          "ee",
        });
        auto lip_oh = ex->Get(libvrm::ExpressionPreset::oh);
        s_labels.push_back({
          VSlider(
            i++, "oh", Enable(lip_oh) ? &lip_oh->weight : nullptr, lipsync),
          "oh",
        });
        // blink
        auto blink = SliderColor::FromHue(4.0f / 7);
        auto blink_LR = ex->Get(libvrm::ExpressionPreset::blink);
        s_labels.push_back({
          VSlider(i++,
                  "blink",
                  Enable(blink_LR) ? &blink_LR->weight : nullptr,
                  blink),
          "--",
        });
        auto blink_L = ex->Get(libvrm::ExpressionPreset::blinkLeft);
        s_labels.push_back({
          VSlider(i++,
                  "blinkLeft",
                  Enable(blink_L) ? &blink_L->weight : nullptr,
                  blink),
          "󰈈-",
        });
        auto blink_R = ex->Get(libvrm::ExpressionPreset::blinkRight);
        s_labels.push_back({
          VSlider(i++,
                  "blinkRight",
                  Enable(blink_R) ? &blink_R->weight : nullptr,
                  blink),
          "-󰈈",
        });
        // lookat
        auto lookat = SliderColor::FromHue(6.0f / 7);
        auto look_up = ex->Get(libvrm::ExpressionPreset::lookUp);
        s_labels.push_back({
          VSlider(i++,
                  "lookUp",
                  Enable(look_up) ? &look_up->weight : nullptr,
                  lookat),
          "󰈈",
        });
        auto look_down = ex->Get(libvrm::ExpressionPreset::lookDown);
        s_labels.push_back({
          VSlider(i++,
                  "lookDown",
                  Enable(look_down) ? &look_down->weight : nullptr,
                  lookat),
          "󰈈",
        });
        auto look_right = ex->Get(libvrm::ExpressionPreset::lookRight);
        s_labels.push_back({
          VSlider(i++,
                  "lookRight",
                  Enable(look_right) ? &look_right->weight : nullptr,
                  lookat),
          "󰈈",
        });
        auto look_left = ex->Get(libvrm::ExpressionPreset::lookLeft);
        s_labels.push_back({
          VSlider(i++,
                  "lookLeft",
                  Enable(look_left) ? &look_left->weight : nullptr,
                  lookat),
          "󰈈",
        });

        ImGui::PopID();
        ImGui::PopStyleVar();

        auto drawList = ImGui::GetWindowDrawList();
        auto cursor = ImGui::GetCursorScreenPos();
        for (auto& label : s_labels) {
          drawList->AddText(
            { label.X, cursor.y }, IM_COL32_BLACK, label.Label, nullptr);
        }

        auto textSize = ImGui::CalcTextSize(" ");
        auto lineheight = textSize.y;
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + lineheight);
      }
    }
  }

  // [vrm]
  // meta
  // humanoid
  // expression
  // lookat
  // firstperson
  // spring
  // constraint
  //
  void Show()
  {
    // switch (m_scene->m_type) {
    //   case libvrm::ModelType::Gltf:
    //     break;
    //   case libvrm::ModelType::Vrm0:
    //     ImGui::Text("%s", "vrm-0.x");
    //     break;
    //   case libvrm::ModelType::Vrm1:
    //     ImGui::Text("%s", "vrm-1.0");
    //     break;
    // }

    if (ImGui::CollapsingHeader("meta", ImGuiTreeNodeFlags_None)) {
    }
    if (ImGui::CollapsingHeader("humanoid", ImGuiTreeNodeFlags_None)) {
    }
    if (ImGui::CollapsingHeader("expression", ImGuiTreeNodeFlags_None)) {
      ShowExpression();
    }
    if (ImGui::CollapsingHeader("lookat", ImGuiTreeNodeFlags_None)) {
    }
    if (ImGui::CollapsingHeader("firstperson", ImGuiTreeNodeFlags_None)) {
    }
    if (ImGui::CollapsingHeader("spring", ImGuiTreeNodeFlags_None)) {
    }
    if (ImGui::CollapsingHeader("constraint", ImGuiTreeNodeFlags_None)) {
    }
  }
};

void
VrmDock::CreateVrm(const AddDockFunc& addDock,
                   std::string_view title,
                   const std::shared_ptr<libvrm::RuntimeScene>& scene)
{
  auto gui = std::make_shared<VrmGui>(scene);

  addDock(grapho::imgui::Dock(title, [gui]() { gui->Show(); }));
}
