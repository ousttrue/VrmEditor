#pragma once
#include "gui.h"
#include <functional>
#include <imgui.h>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// Usage:
//  static ExampleAppLog my_log;
//  my_log.AddLog("Hello %d world\n", 123);
//  my_log.Draw("title");
enum class LogLevel
{
  // green
  Info,
  // gray
  Debug,
  // orange
  Wran,
  // red
  Error,
};
struct Msg
{
  LogLevel Level;
  std::string Message;
};

using AddLogFunc = std::function<void(std::string_view)>;
using FlushLog = std::function<void()>;

class ImLogger
{
  std::vector<Msg> Logs;
  // ImGuiTextFilter Filter;
  // ImVector<int>
  //   LineOffsets; // Index to lines offset. We maintain this with AddLog()
  //   calls.
  bool AutoScroll = true;

public:
  ImLogger() { Clear(); }

  static void Create(const AddDockFunc& addDock,
                     const std::shared_ptr<ImLogger>& logger)
  {
    addDock(Dock("logger", [logger]() { logger->Draw(); }));
  }

  void Clear()
  {
    Logs.clear();
    // LineOffsets.clear();
    // LineOffsets.push_back(0);
  }

  LogLevel m_level;
  std::stringstream m_ss;
  std::ostream& Begin(LogLevel level)
  {
    m_level = level;
    return m_ss;
  }
  void Push(std::string_view msg) { m_ss << msg; }
  void End()
  {
    auto str = m_ss.str();
    m_ss.str("");
    if (str.size()) {
      Logs.push_back({ .Level = m_level, .Message = str });
    }
  }

  void AddLog(LogLevel level, std::string_view msg)
  {
    // int old_size = Buf.size();
    // va_list args;
    // va_start(args, fmt);
    // Buf.appendfv(fmt, args);

    Begin(level);
    Push(msg);
    End();

    // for (int new_size = Logs.size(); old_size < new_size; old_size++)
    //   if (Buf[old_size] == '\n')
    //     LineOffsets.push_back(old_size + 1);
  }

  void Draw()
  {
    // Options menu
    if (ImGui::BeginPopup("Options")) {
      ImGui::Checkbox("Auto-scroll", &AutoScroll);
      ImGui::EndPopup();
    }

    // Main window
    if (ImGui::Button("Options"))
      ImGui::OpenPopup("Options");
    ImGui::SameLine();
    bool clear = ImGui::Button("Clear");
    ImGui::SameLine();
    bool copy = ImGui::Button("Copy");
    ImGui::SameLine();
    // Filter.Draw("Filter", -100.0f);

    ImGui::Separator();

    if (ImGui::BeginChild("scrolling",
                          ImVec2(0, 0),
                          false,
                          ImGuiWindowFlags_HorizontalScrollbar)) {
      if (clear)
        Clear();
      if (copy)
        ImGui::LogToClipboard();

      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
      for (auto& log : Logs) {
        ImGui::TextUnformatted(log.Message.data(),
                               log.Message.data() + log.Message.size());
      }

      // const char* buf = Buf.begin();
      // const char* buf_end = Buf.end();
      // if (Filter.IsActive()) {
      //   // In this example we don't use the clipper when Filter is enabled.
      //   // This is because we don't have random access to the result of our
      //   // filter. A real application processing logs with ten of thousands
      //   of
      //   // entries may want to store the result of search/filter.. especially
      //   if
      //   // the filtering function is not trivial (e.g. reg-exp).
      //   for (int line_no = 0; line_no < LineOffsets.Size; line_no++) {
      //     const char* line_start = buf + LineOffsets[line_no];
      //     const char* line_end = (line_no + 1 < LineOffsets.Size)
      //                              ? (buf + LineOffsets[line_no + 1] - 1)
      //                              : buf_end;
      //     if (Filter.PassFilter(line_start, line_end))
      //       ImGui::TextUnformatted(line_start, line_end);
      //   }
      // } else {
      //   // The simplest and easy way to display the entire buffer:
      //   //   ImGui::TextUnformatted(buf_begin, buf_end);
      //   // And it'll just work. TextUnformatted() has specialization for
      //   large
      //   // blob of text and will fast-forward to skip non-visible lines. Here
      //   we
      //   // instead demonstrate using the clipper to only process lines that
      //   are
      //   // within the visible area.
      //   // If you have tens of thousands of items and their processing cost
      //   is
      //   // non-negligible, coarse clipping them on your side is recommended.
      //   // Using ImGuiListClipper requires
      //   // - A) random access into your data
      //   // - B) items all being the  same height,
      //   // both of which we can handle since we have an array pointing to the
      //   // beginning of each line of text. When using the filter (in the
      //   block
      //   // of code above) we don't have random access into the data to
      //   display
      //   // anymore, which is why we don't use the clipper. Storing or
      //   skimming
      //   // through the search result would make it possible (and would be
      //   // recommended if you want to search through tens of thousands of
      //   // entries).
      //   ImGuiListClipper clipper;
      //   clipper.Begin(LineOffsets.Size);
      //   while (clipper.Step()) {
      //     for (int line_no = clipper.DisplayStart; line_no <
      //     clipper.DisplayEnd;
      //          line_no++) {
      //       const char* line_start = buf + LineOffsets[line_no];
      //       const char* line_end = (line_no + 1 < LineOffsets.Size)
      //                                ? (buf + LineOffsets[line_no + 1] - 1)
      //                                : buf_end;
      //       ImGui::TextUnformatted(line_start, line_end);
      //     }
      //   }
      //   clipper.End();
      // }
      ImGui::PopStyleVar();

      // Keep up at the bottom of the scroll region if we were already at the
      // bottom at the beginning of the frame. Using a scrollbar or mouse-wheel
      // will take away from the bottom edge.
      if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
  }
};

struct LogStream
{
  std::ostream& m_os;
  FlushLog m_flush;

public:
  LogStream(std::ostream& os, const FlushLog& flush)
    : m_os(os)
    , m_flush(flush)
  {
  }
  ~LogStream() { m_flush(); }
  template<typename T>
  LogStream& operator<<(T msg)
  {
    m_os << msg;
    return *this;
  }
};
