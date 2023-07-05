#include "vrm_gui.h"
#include "springbone_gui.h"
#include <glr/gl3renderer.h>
#include <gltfjson/gltf_typing_vrm0.h>
#include <gltfjson/gltf_typing_vrm1.h>
#include <grapho/imgui/printfbuffer.h>
#include <imgui.h>
#include <json_widgets.h>
#include <vrm/runtime_node.h>

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

static float
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

struct VrmImpl
{
  ClearJsonPathFunc m_clear;
  std::shared_ptr<libvrm::RuntimeScene> m_runtime;
  std::shared_ptr<libvrm::Image> m_thumbImage;
  std::shared_ptr<gui::SpringBoneGui> m_springbone;

  VrmImpl(const ClearJsonPathFunc& clear)
    : m_clear(clear)
    , m_springbone(new gui::SpringBoneGui)
  {
  }

  void SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime)
  {
    m_runtime = runtime;
    if (auto VRMC_vrm =
          m_runtime->m_base->m_gltf->GetExtension<gltfjson::vrm1::VRMC_vrm>()) {
      if (auto meta = VRMC_vrm->Meta()) {
        if (auto imgId = meta->ThumbnailImageId()) {
          m_thumbImage = glr::GetOrCreateImage(
            *m_runtime->m_base->m_gltf, m_runtime->m_base->m_bin, *imgId);
        }
      }
    } else if (auto VRM = m_runtime->m_base->m_gltf
                            ->GetExtension<gltfjson::vrm0::VRM>()) {
      if (auto meta = VRM->Meta()) {
        if (auto texId = meta->TextureId()) {
          auto texture = m_runtime->m_base->m_gltf->Textures[*texId];
          if (auto imgId = texture.SourceId()) {
            m_thumbImage = glr::GetOrCreateImage(
              *m_runtime->m_base->m_gltf, m_runtime->m_base->m_bin, *imgId);
          }
        }
      }
    }

    m_springbone->SetRuntime(runtime);
  }

  void ShowMeta()
  {
    if (!m_runtime) {
      return;
    }

    if (m_thumbImage) {
      auto tex =
        glr::GetOrCreateTextureHandle(m_thumbImage, glr::ColorSpace::Linear);
      ImGui::Image((ImTextureID)(intptr_t)tex.value_or(0), { 100, 100 });
    }

    if (auto VRMC_vrm =
          m_runtime->m_base->m_gltf->GetExtension<gltfjson::vrm1::VRMC_vrm>()) {
      ImGui::TextUnformatted("vrm-1.0");
      if (auto meta = VRMC_vrm->Meta()) {
        if (ShowGuiVrm1Meta(*meta)) {
          m_clear(u8"/extensions/VRMC_vrm/meta");
        }
      }
    } else if (auto VRM = m_runtime->m_base->m_gltf
                            ->GetExtension<gltfjson::vrm0::VRM>()) {
      ImGui::TextUnformatted("vrm-0.x");
      if (auto meta = VRM->Meta()) {
        if (ShowGuiVrm0Meta(*meta)) {
          m_clear(u8"/extensions/VRM/meta");
        }
      }
    }
  }

  void ShowExpression(const char* label, libvrm::Expression& expression)
  {
    ImGui::BeginDisabled(expression.Empty());
    ImGui::SliderFloat(label, &expression.weight, 0, 1);
    ImGui::EndDisabled();
  }

  void ShowExpressionEdit(libvrm::Expressions& expressions) {}

  void ShowExpressionRuntime(libvrm::Expressions& expressions)
  {
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader("emotion")) {
      ShowExpression("😆Happy", expressions.Happy);
      ShowExpression("😠Angry", expressions.Angry);
      ShowExpression("😥Sad", expressions.Sad);
      ShowExpression("🙂Relaxed", expressions.Relaxed);
      ShowExpression("😯Surprised", expressions.Surprised);
    }
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader("blink")) {
      ShowExpression("😉Blink", expressions.Blink);
      ShowExpression("😉BlinkLeft", expressions.BlinkLeft);
      ShowExpression("😉BlinkRight", expressions.BlinkRight);
    }
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader("lipsync")) {
      ShowExpression("👄Aa", expressions.Aa);
      ShowExpression("👄Ih", expressions.Ih);
      ShowExpression("👄Ou", expressions.Ou);
      ShowExpression("👄Ee", expressions.Ee);
      ShowExpression("👄Oh", expressions.Oh);
    }
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader("lookat")) {
      ShowExpression("👀LookUp", expressions.LookUp);
      ShowExpression("👀LookDown", expressions.LookDown);
      ShowExpression("👀LookLeft", expressions.LookLeft);
      ShowExpression("👀LookRight", expressions.LookRight);
    }
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader("other")) {
      ShowExpression("😶Neutral", expressions.Neutral);
    }
    if (ImGui::CollapsingHeader("custom")) {
      for (auto& custom : expressions.CustomExpressions) {
        ShowExpression(custom.name.c_str(), custom);
      }
    }
  }

  // { { u8"😆", u8"happy" } },
  // { { u8"😠", u8"angry" } },
  // { { u8"😥", u8"sad" } },
  // { { u8"🙂", u8"relaxed" } },
  // { { u8"😯", u8"surprised" } },
  // { { u8"👄", u8"aa" } },
  // { { u8"👄", u8"ih" } },
  // { { u8"👄", u8"ou" } },
  // { { u8"👄", u8"ee" } },
  // { { u8"👄", u8"oh" } },
  // { { u8"😉", u8"blink" } },
  // { { u8"😉", u8"blinkLeft" } },
  // { { u8"😉", u8"blinkRight" } },
  // { { u8"👀", u8"lookUp" } },
  // { { u8"👀", u8"lookDown" } },
  // { { u8"👀", u8"lookLeft" } },
  // { { u8"👀", u8"lookRight" } },
  // { { u8"😶", u8"neutral" } },
  void ShowExpression(libvrm::Expressions& expressions)
  {
    if (ImGui::BeginTabBar("Expression")) {
      if (ImGui::BeginTabItem("🎁Asset")) {
        ShowExpressionEdit(expressions);
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("🎬Runtime")) {
        ShowExpressionRuntime(expressions);
        ImGui::EndTabItem();
      }

      ImGui::EndTabBar();
    }
  }

  // [vrm]
  // { { u8"🪪", u8"meta" } },
  // { { u8"👤", u8"humanoid" } },
  // { { u8"👀", u8"firstPerson" } },
  // { { u8"😀", u8"blendShapeMaster" } },
  // { { u8"🔗", u8"secondaryAnimation" } },
  // { { u8"💎", u8"materialProperties" } },
  void ShowGui()
  {
    if (!m_runtime) {
      return;
    }

    if (ImGui::BeginTabBar("Vrm")) {
      if (ImGui::BeginTabItem("🪪Meta")) {
        ShowMeta();
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("😀Expression")) {
        if (auto ex = m_runtime->m_expressions) {
          ShowExpression(*ex);
        }
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("👀LookAt")) {
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("🔗Spring")) {
        m_springbone->ShowGui();
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Constraint")) {
        ImGui::EndTabItem();
      }

      ImGui::EndTabBar();
    }
  }
};

VrmGui::VrmGui(const ClearJsonPathFunc& clear)
  : m_impl(new VrmImpl(clear))
{
}

VrmGui::~VrmGui()
{
  delete m_impl;
}

void
VrmGui::SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime)
{
  m_impl->SetRuntime(runtime);
}

void
VrmGui::ShowGui()
{
  m_impl->ShowGui();
}
