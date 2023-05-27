#pragma once
#include "showgui.h"
#include <array>
#include <functional>
#include <imgui.h>
#include <string>
#include <vector>

inline bool
GuiTable(const char* title, std::span<const char*> cols)
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

template<typename T>
struct TableColumn
{
  std::string Header;
  std::function<void(size_t index, const T& value)> Callback;
};

template<typename T>
inline ShowGuiFunc
TableToShowGui(const char* name,
               std::span<const TableColumn<T>> cols,
               const std::vector<T>& values)
{
  return [name, values, cols]() {
    std::vector<const char*> headers;
    for (auto& col : cols) {
      headers.push_back(col.Header.c_str());
    }
    if (GuiTable(name, headers)) {
      for (size_t y = 0; y < values.size(); ++y) {
        ImGui::TableNextRow();
        int x = 0;
        for (auto& col : cols) {
          ImGui::TableSetColumnIndex(x++);
          col.Callback(y, values[y]);
        }
      }
      ImGui::EndTable();
    }
  };
}
