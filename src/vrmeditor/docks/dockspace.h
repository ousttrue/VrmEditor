#pragma once
#include <filesystem>
#include <functional>
#include <grapho/imgui/dockspace.h>
#include <list>
#include <vector>

using AddDockFunc = std::function<void(const grapho::imgui::Dock& dock)>;

struct DockOptions
{
  bool ShowDefault = false;
  bool Temporary = false;
  bool Debug = false;
};

struct DockSpaceManager
{
  std::list<grapho::imgui::Dock> Docks;
  bool m_resetLayout = true;
  std::filesystem::path m_fileDialogCurrent;

  std::list<grapho::imgui::Dock> TmpDocks;
  std::list<grapho::imgui::Dock> DebugDocks;

  std::list<std::function<void()>> OnResetCallbacks;

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

  void Reset();
  grapho::imgui::Dock& AddDock(const grapho::imgui::Dock& dock,
                               const DockOptions& options = {});
  void SetDockVisible(std::string_view name, bool visible);
  void ShowGui();
};
