#include "overlay.h"
#include <imgui.h>
#include <stdio.h>

namespace glr {

void
Overlay(const ImVec2& pos,
        const char* title,
        const char* popupName,
        const std::function<void()>& popup)
{
  auto backup = ImGui::GetCursorPos();

  ImGuiWindowFlags window_flags =
    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, { 1, 0 });
  window_flags |= ImGuiWindowFlags_NoMove;
  ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
  char buf[64];
  snprintf(buf, sizeof(buf), " %s ", title);
  auto textSize = ImGui::CalcTextSize(buf);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
  if (ImGui::BeginChild(
        "overlay", { textSize.x, textSize.y * 2 }, false, window_flags)) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + textSize.y * 0.5f);
    ImGui::TextColored({ 1, 1, 1, 1 }, "%s", buf);
    if (popupName && popup) {
      popup();
      ImGui::OpenPopupOnItemClick(popupName,
                                  ImGuiPopupFlags_MouseButtonLeft |
                                    ImGuiPopupFlags_MouseButtonRight);
    }
  }

  ImGui::EndChild();
  ImGui::PopStyleVar();

  ImGui::SetCursorPos(backup);
}

}
