#include "imlogger.h"
#include "fs_util.h"
#include <array>
#include <filesystem>
#include <grapho/imgui/widgets.h>
#include <list>
#include <regex>

ImLogger::ImLogger()
{
  Clear();

  std::filesystem::path home = get_env("USERPROFILE");

  std::list<std::string> separate;
  for (auto current = home;; current = current.parent_path()) {
    separate.push_front(current.filename().string());
    if (current.parent_path() == current) {
      break;
    }
  }

  std::string pattern;
  pattern += home.root_name().string();
  for (auto& s : separate) {
    if (s.empty()) {
      continue;
    }
    pattern += R"([\\/])";
    pattern += s;
  }

  m_home = std::regex((const char*)pattern.c_str());
}

void
ImLogger::Clear()
{
  Logs.clear();
}

static std::string
SubstitutePath(const std::string& text, const std::regex& re)
{
  std::smatch match;
  if (!std::regex_search(text, match, re)) {
    return text;
  }

  auto last = text.begin();
  std::string result;
  for (int i = 0; i < match.size(); ++i) {
    result += std::string(last, match[i].first);
    result += "~";
    last = match[i].second;
  }
  result += std::string(last, text.end());

  return result;
}

void
ImLogger::AddLog(plog::Severity level, const plog::util::nstring& msg)
{
  plog::util::nstring mod = SubstitutePath(msg.c_str(), m_home);
  Logs.push_back({ .Level = level, .Message = mod });
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
  if (ImGui::Button("Clear")) {
    Clear();
  }

  ImGui::Separator();

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
        ImGui::TableNextColumn();
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
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ImGui::GetColorU32({ 0.2f, 0.2f, 0.2f, 1 }));
            ImGui::TextUnformatted("DEBUG");
            break;
          case plog::Severity::info:
            // green
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ImGui::GetColorU32({ 0.2f, 0.5f, 0.2f, 1 }));
            ImGui::TextUnformatted("INFO");
            break;
          case plog::Severity::warning:
            // orange
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ImGui::GetColorU32({ 0.5f, 0.4f, 0.2f, 1 }));
            ImGui::TextUnformatted("WARN");
            break;
          case plog::Severity::fatal:
          case plog::Severity::error:
            // red
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                   ImGui::GetColorU32({ 0.5f, 0.2f, 0.2f, 1 }));
            ImGui::TextUnformatted("ERROR");
            break;
        }

        // 1
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(log.Message.data(),
                               log.Message.data() + log.Message.size());
      }
    }

    // Keep up at the bottom of the scroll region if we were already at the
    // bottom at the beginning of the frame. Using a scrollbar or mouse-wheel
    // will take away from the bottom edge.
    if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
      ImGui::SetScrollHereY(1.0f);

    ImGui::EndTable();
  }
}
