#pragma once
#include "gui.h"
#include "gui_table.h"
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
                     std::string_view title,
                     const std::shared_ptr<ImLogger>& logger)
  {
    addDock(grapho::imgui::Dock(
      title, [logger]() { logger->Draw(); }, true));
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

      std::array<const char*, 2> cols = {
        "level",
        "message",
      };
      if (GuiTable("##log_table", cols)) {
        ImGuiListClipper clipper;
        clipper.Begin(Logs.size());
        while (clipper.Step()) {
          for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
            auto& log = Logs[i];
            ImGui::TableNextRow();
            // 0

            ImGui::TableSetColumnIndex(0);
            switch (log.Level) {
              case LogLevel::Debug:
                // gray
                ImGui::TableSetBgColor(
                  ImGuiTableBgTarget_CellBg,
                  ImGui::GetColorU32({ 0.2f, 0.2f, 0.2f, 1 }));
                ImGui::TextUnformatted("DEBUG");
                break;
              case LogLevel::Info:
                // green
                ImGui::TableSetBgColor(
                  ImGuiTableBgTarget_CellBg,
                  ImGui::GetColorU32({ 0.2f, 0.5f, 0.2f, 1 }));
                ImGui::TextUnformatted("INFO");
                break;
              case LogLevel::Wran:
                // orange
                ImGui::TableSetBgColor(
                  ImGuiTableBgTarget_CellBg,
                  ImGui::GetColorU32({ 0.5f, 0.4f, 0.2f, 1 }));
                ImGui::TextUnformatted("WARN");
                break;
              case LogLevel::Error:
                // red
                ImGui::TableSetBgColor(
                  ImGuiTableBgTarget_CellBg,
                  ImGui::GetColorU32({ 0.5f, 0.2f, 0.2f, 1 }));
                ImGui::TextUnformatted("ERROR");
                break;
            }
            // 1
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(log.Message.data(),
                                   log.Message.data() + log.Message.size());
          }
        }
        ImGui::EndTable();
      }

      // // ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
      // for (auto& log : Logs) {
      // }

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
