#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS 1
#include <chrono>
#include <imgui.h>
#include <imgui_internal.h>

// cursor
// start      end
// +-----------+
// |           |
// +-----------+
struct TimeDraw {
  ImVec2 cursor;
  ImVec2 size;
  std::chrono::milliseconds start;
  std::chrono::milliseconds end;
  ImU32 color = IM_COL32_BLACK;
  TimeDraw(const ImVec2 &cursor, const ImVec2 &size,
           std::chrono::milliseconds start, std::chrono::milliseconds end)
      : cursor(cursor), size(size), start(start), end(end) {}
  TimeDraw(const ImVec2 &cursor, const ImVec2 &size,
           std::chrono::milliseconds center)
      : TimeDraw(cursor, size,
                 center - std::chrono::milliseconds((int64_t)size.x * 5),
                 center + std::chrono::milliseconds((int64_t)size.x * 5)) {}
  bool drawLine(ImDrawList *drawList, std::chrono::milliseconds time) {

    if (time < start) {
      return false;
    }
    if (time > end) {
      return false;
    }

    auto delta = time - start;
    auto x = cursor.x + delta.count() * 0.1f;
    drawList->AddLine({x, cursor.y}, {x, cursor.y + size.y}, color, 1.0f);

    char text[64];
    sprintf_s(text, sizeof(text), "%ld", time.count());
    drawList->AddText({x, cursor.y}, color, text, nullptr);

    return true;
  }

  void drawLines(ImDrawList *drawList) {
    auto count = start.count() / 1000;
    for (auto current = std::chrono::milliseconds(count * 1000); current < end;
         current += std::chrono::milliseconds(1000)) {
      drawLine(drawList, current);
    }
  }

  void drawNow(ImDrawList *drawList, std::chrono::milliseconds time) {
    auto delta = time - start;
    auto x = cursor.x + delta.count() * 0.1f;
    drawList->AddLine({x, cursor.y}, {x, cursor.y + size.y},
                      IM_COL32(255, 0, 0, 255), 1.0f);

    char text[64];
    sprintf_s(text, sizeof(text), "%ld", time.count());
    drawList->AddText({x, cursor.y}, color, text, nullptr);
  }
};

class ImTimeline {
public:
  ImTimeline() {}
  ~ImTimeline() {}

  void show(std::chrono::milliseconds now, const ImVec2 &size = {0, 0}) {
    // ImGuiContext &g = *GImGui;
    // ImGuiWindow *window = ImGui::GetCurrentWindow();
    // const auto &imStyle = ImGui::GetStyle();
    const auto drawList = ImGui::GetWindowDrawList();
    const auto cursor = ImGui::GetCursorScreenPos();
    const auto area = ImGui::GetContentRegionAvail();
    // const auto cursorBasePos = ImGui::GetCursorScreenPos() + window->Scroll;

    auto textSize = ImGui::CalcTextSize(" ");
    auto lineheight = textSize.y;

    auto realSize = ImFloor(size);
    if (realSize.x <= 0.0f)
      realSize.x = ImMax(4.0f, area.x);
    if (realSize.y <= 0.0f)
      realSize.y = ImMax(4.0f, lineheight);

    TimeDraw(cursor, {realSize.x, lineheight + lineheight}, now)
        .drawNow(drawList, now);

    TimeDraw({cursor.x, cursor.y + lineheight}, realSize, now)
        .drawLines(drawList);
  }
};
