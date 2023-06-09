#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS 1
#include "gui.h"
#include <chrono>
#include <imgui.h>
#include <imgui_internal.h>
#include <math.h>
#include <memory>
#include <sstream>
#include <vrm/timeline.h>

// cursor
// start      end
// +-----------+
// |           |
// +-----------+
struct TimeGeometry
{
  ImVec2 Cursor;
  ImVec2 Size;
  libvrm::Time Start;
  libvrm::Time End;
  ImU32 Color = IM_COL32_BLACK;
  TimeGeometry(const ImVec2& cursor,
               const ImVec2& size,
               libvrm::Time start,
               libvrm::Time end)
    : Cursor(cursor)
    , Size(size)
    , Start(start)
    , End(end)
  {
  }

  float GetX(libvrm::Time time)
  {
    auto delta = time - Start;
    auto x = Cursor.x + (float)delta.count() * 100;
    return x;
  }

  // 1seconds / 100 pixel
  // pixels to seconds is 0.01
  // half 0.005
  TimeGeometry(const ImVec2& cursor, const ImVec2& size, libvrm::Time center)
    : TimeGeometry(cursor,
                   size,
                   center - libvrm::Time((double)size.x * 0.005),
                   center + libvrm::Time((double)size.x * 0.005))
  {
  }

  bool DrawLine(ImDrawList* drawList, libvrm::Time time)
  {

    if (time < Start) {
      return false;
    }
    if (time > End) {
      return false;
    }

    auto x = GetX(time);
    drawList->AddLine({ x, Cursor.y }, { x, Cursor.y + Size.y }, Color, 1.0f);

    char text[64];
    auto len = snprintf(text, sizeof(text), "%.2lf", time.count());
    drawList->AddText({ x, Cursor.y }, Color, text, text + len);

    return true;
  }

  void DrawLines(ImDrawList* drawList)
  {
    auto count = (int)Start.count();
    for (auto current = libvrm::Time(count); current < End;
         current += libvrm::Time(1.0)) {
      DrawLine(drawList, current);
    }
  }

  float DrawNow(ImDrawList* drawList, libvrm::Time time)
  {
    auto x = GetX(time);

    char text[64];
    auto len = snprintf(text, sizeof(text), "%.2lf", time.count());
    drawList->AddText({ x, Cursor.y }, Color, text, text + len);

    return x;
  }
};

class ImTimeline
{
public:
  static void Create(const AddDockFunc& addDock,
                     std::string_view title,
                     const std::shared_ptr<libvrm::Timeline>& timeline)
  {
    auto timelineGui = std::make_shared<ImTimeline>();
    addDock({
      .Name = { title.begin(), title.end() },
      .OnShow = [timeline, timelineGui]() { timelineGui->show(timeline); },
    });
  }

  void show(const std::shared_ptr<libvrm::Timeline>& timeline,
            const ImVec2& size = { 0, 0 })
  {
    ImGui::Checkbox("IsPlaying", &timeline->IsPlaying);
    ImGui::BeginDisabled(timeline->IsPlaying);
    if (ImGui::Button("next frame")) {
      timeline->SetDeltaTime(libvrm::Time(1.0 / 60), true);
    }
    ImGui::EndDisabled();

    // ImGuiContext &g = *GImGui;
    // ImGuiWindow *window = ImGui::GetCurrentWindow();
    // const auto &imStyle = ImGui::GetStyle();
    const auto drawList = ImGui::GetWindowDrawList();
    auto cursor = ImGui::GetCursorScreenPos();
    const float top = cursor.y;
    const auto area = ImGui::GetContentRegionAvail();
    // const auto cursorBasePos = ImGui::GetCursorScreenPos() + window->Scroll;

    auto textSize = ImGui::CalcTextSize(" ");
    auto lineheight = textSize.y;

    auto realSize = ImFloor(size);
    if (realSize.x <= 0.0f)
      realSize.x = ImMax(4.0f, area.x);
    if (realSize.y <= 0.0f)
      realSize.y = ImMax(4.0f, lineheight);

    auto now = timeline->CurrentTime;
    auto nowX =
      TimeGeometry(cursor, { realSize.x, lineheight + lineheight }, now)
        .DrawNow(drawList, now);
    cursor.y += lineheight;

    TimeGeometry(cursor, realSize, now).DrawLines(drawList);
    cursor.y += lineheight;

    for (auto& track : timeline->Tracks) {
      if (track->Duration.count() == 0) {
        continue;
      }
      TimeGeometry draw(cursor, realSize, now);
      if (auto startTime = track->StartTime) {
        auto left = draw.GetX(*startTime);
        auto right = draw.GetX(*startTime + track->Duration);
        if (track->Loop) {
          auto width = right - left;
          int count = static_cast<int>(left / width);
          auto x = left + width * count;
          for (; x > cursor.x; x -= width) {
          }
          for (; x + width < cursor.x; x += width) {
          }
          for (; x < cursor.x + area.x; x += width) {
            drawList->AddRectFilled({ x, cursor.y },
                                    { x + width, cursor.y + lineheight },
                                    IM_COL32_WHITE);
            drawList->AddLine(
              { x, cursor.y }, { x, cursor.y + lineheight }, IM_COL32_BLACK);
          }
        } else {
          drawList->AddRectFilled({ left, cursor.y },
                                  { right, cursor.y + lineheight },
                                  IM_COL32_WHITE);
        }
      } else {
        // not playing
      }
      // track name
      drawList->AddText(cursor, IM_COL32_BLACK, track->Name.c_str(), nullptr);

      cursor.y += lineheight;
    }

    // center line
    drawList->AddLine(
      { nowX, top }, { nowX, cursor.y }, IM_COL32(255, 0, 0, 255), 1.0f);

    ImGui::SetCursorScreenPos(cursor);
  }
};
