#pragma once
#include <filesystem>
#include <functional>
#include <grapho/imgui/dockspace.h>
#include <list>
#include <vector>

using AddDockFunc = std::function<void(const grapho::imgui::Dock& dock)>;

struct DockSpaceManager
{
  std::list<grapho::imgui::Dock> Docks;
  bool m_resetLayout = true;
  std::filesystem::path m_fileDialogCurrent;

  std::list<grapho::imgui::Dock> TmpDocks;

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

  void AddDock(const grapho::imgui::Dock& dock, bool tmporary = false);
  void SetDockVisible(std::string_view name, bool visible);
  void ShowGui();
};
