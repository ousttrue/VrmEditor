#pragma once
#include <imgui.h>

inline bool
JsonGuiTable(const char* title, std::span<const char*> cols)
{
  static ImGuiTableFlags flags =
    ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
    ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody |
    ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
    ImGuiTableFlags_SizingFixedFit;

  if (ImGui::BeginTable(title, cols.size(), flags)) {
    for (auto col : cols) {
      ImGui::TableSetupColumn(col);
    }
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableHeadersRow();
    return true;
  }

  return false;
}
