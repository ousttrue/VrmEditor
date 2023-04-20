#include "overlay.h"
#include <imgui.h>
#include <stdio.h>

namespace glr {
void
Overlay(const ImVec2& pos, const char* text)
{
  auto backup = ImGui::GetCursorPos();

  // ImGuiIO& io = ImGui::GetIO();
  ImGuiWindowFlags window_flags =
    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
  // if (location >= 0)
  {
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, { 1, 0 });
    window_flags |= ImGuiWindowFlags_NoMove;
  }
  ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
  char buf[64];
  snprintf(buf, sizeof(buf), " %s ", text);
  auto textSize = ImGui::CalcTextSize(buf);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
  if (ImGui::BeginChild(
        "overlay", { textSize.x, textSize.y * 2 }, false, window_flags)) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + textSize.y * 0.5f);
    ImGui::TextColored({ 1, 1, 1, 1 }, "%s", buf);
  }
  ImGui::EndChild();
  ImGui::PopStyleVar();

  ImGui::SetCursorPos(backup);
}
}
