#include "imlogger.h"
#include "windows_helper.h"
#include <array>
#include <grapho/imgui/widgets.h>

ImLogger::ImLogger()
{
  Clear();

  m_home = GetEnv("USERPROFILE");
}

// void
// ImLogger::CreateDock(const AddDockFunc& addDock, std::string_view title)
// {
//   addDock(grapho::imgui::Dock(
//     title, [=]() { Draw(); }, true));
// }

void
ImLogger::Clear()
{
  Logs.clear();
  // LineOffsets.clear();
  // LineOffsets.push_back(0);
}

// std::string
// ImLogger::SubStitute(std::string_view src)
// {
//   auto pos = src.find(m_home);
//   if (pos == std::string::npos) {
//     return { src.data(), src.size() };
//   }
//
//   std::string dst(src.data(), pos);
//   dst.push_back('~');
//   dst += src.substr(pos + m_home.size());
//   return dst;
// }
//
// void
// ImLogger::End()
// {
//   std::string str;
//   // m_ss >> std::quoted(str);
//   str = m_ss.str();
//   m_ss.str("");
//   if (str.size()) {
//
//     str = SubStitute(str);
//
//     Logs.push_back({ .Level = m_level, .Message = str });
//   }
// }

void
ImLogger::AddLog(plog::Severity level, const plog::util::nstring& msg)
{
  // int old_size = Buf.size();
  // va_list args;
  // va_start(args, fmt);
  // Buf.appendfv(fmt, args);

  // Begin(level);
  // Push(msg);
  // End();
  Logs.push_back({ .Level = level, .Message = msg });

  // for (int new_size = Logs.size(); old_size < new_size; old_size++)
  //   if (Buf[old_size] == '\n')
  //     LineOffsets.push_back(old_size + 1);
}

void
ImLogger::Draw()
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

  auto size = ImGui::GetContentRegionAvail();
  if (ImGui::BeginChild(
        "scrolling", size, false, ImGuiWindowFlags_HorizontalScrollbar)) {
    if (clear)
      Clear();
    if (copy)
      ImGui::LogToClipboard();

    std::array<const char*, 2> cols = {
      "level",
      "message",
    };
    if (grapho::imgui::BeginTableColumns("##log_table", cols)) {
      ImGuiListClipper clipper;
      clipper.Begin(Logs.size());
      while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
          auto& log = Logs[i];
          ImGui::TableNextRow();
          // 0

          ImGui::TableSetColumnIndex(0);
          // none = 0,
          // fatal = 1,
          // error = 2,
          // warning = 3,
          // info = 4,
          // debug = 5,
          // verbose = 6
          switch (log.Level) {
            case plog::Severity::none:
            case plog::Severity::verbose:
            case plog::Severity::debug:
              // gray
              ImGui::TableSetBgColor(
                ImGuiTableBgTarget_CellBg,
                ImGui::GetColorU32({ 0.2f, 0.2f, 0.2f, 1 }));
              ImGui::TextUnformatted("DEBUG");
              break;
            case plog::Severity::info:
              // green
              ImGui::TableSetBgColor(
                ImGuiTableBgTarget_CellBg,
                ImGui::GetColorU32({ 0.2f, 0.5f, 0.2f, 1 }));
              ImGui::TextUnformatted("INFO");
              break;
            case plog::Severity::warning:
              // orange
              ImGui::TableSetBgColor(
                ImGuiTableBgTarget_CellBg,
                ImGui::GetColorU32({ 0.5f, 0.4f, 0.2f, 1 }));
              ImGui::TextUnformatted("WARN");
              break;
            case plog::Severity::fatal:
            case plog::Severity::error:
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
