#include "gizmo_settings.h"
#include <ImGuizmo.h>
#include <ImGuizmoContext.h>

struct GizmoSettingsImpl
{
  GizmoSettingsImpl()
  {
    auto& style = ImGuizmo::GetStyle();
    style.TranslationLineArrowSize = 15;
    style.TranslationLineThickness = 10;
    style.RotationLineThickness = 10;
    style.RotationOuterLineThickness = 10;
    ImGuizmo::Context::Instance().mGizmoSizeClipSpace = 0.2f;
  }

  void ShowGui()
  {
    ImGui::SliderFloat(
      "gizmo size", &ImGuizmo::GetContext().mGizmoSizeClipSpace, 0.01f, 1.0f);

    ImGui::Separator();

    // float TranslationLineThickness;   // Thickness of lines for translation
    // gizmo float TranslationLineArrowSize;   // Size of arrow at the end of
    // lines for
    //                                   // translation gizmo
    //                                   // rotation gizmo
    // float ScaleLineThickness;         // Thickness of lines for scale gizmo
    // float
    //   ScaleLineCircleSize; // Size of circle at the end of lines for scale
    //   gizmo
    // float HatchedAxisLineThickness; // Thickness of hatched axis lines
    // float CenterCircleSize; // Size of circle at the center of the
    // translate/scale
    //                         // gizmo

    ImGui::SliderFloat("translation line",
                       &ImGuizmo::GetStyle().TranslationLineThickness,
                       1.0f,
                       30.0f);
    ImGui::SliderFloat("translation arrow size",
                       &ImGuizmo::GetStyle().TranslationLineArrowSize,
                       1.0f,
                       30.0f);

    ImGui::SliderFloat("rotation line",
                       &ImGuizmo::GetStyle().RotationLineThickness,
                       1.0f,
                       30.0f);
    ImGui::SliderFloat("rotation outer line",
                       &ImGuizmo::GetStyle().RotationOuterLineThickness,
                       1.0f,
                       30.0f);
  }
};

GizmoSettings::GizmoSettings()
  : m_impl(new GizmoSettingsImpl)
{
}

GizmoSettings::~GizmoSettings()
{
  delete m_impl;
}

void
GizmoSettings::ShowGui()
{
  m_impl->ShowGui();
}
