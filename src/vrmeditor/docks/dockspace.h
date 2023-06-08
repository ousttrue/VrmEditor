#pragma once
#include <filesystem>
#include <functional>
#include <grapho/imgui/dockspace.h>
#include <vector>

using AddDockFunc = std::function<void(const grapho::imgui::Dock& dock)>;

struct DockSpaceManager
{
  std::vector<grapho::imgui::Dock> Docks;
  bool m_resetLayout = true;
  std::filesystem::path m_fileDialogCurrent;

private:
  DockSpaceManager();

public:
  ~DockSpaceManager() {}
  DockSpaceManager(const DockSpaceManager&) = delete;
  DockSpaceManager& operator=(const DockSpaceManager&) = delete;
  static DockSpaceManager& Instance()
  {
    static DockSpaceManager s_instance;
    return s_instance;
  }

  void AddDock(const grapho::imgui::Dock& dock);
  void SetDockVisible(std::string_view name, bool visible);
  void ShowGui();
};
