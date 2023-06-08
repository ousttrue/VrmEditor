#pragma once
#include <filesystem>
#include <functional>
#include <grapho/imgui/dockspace.h>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

using AddDockFunc = std::function<void(const grapho::imgui::Dock& dock)>;

class Gui
{
  struct GuiImpl* m_impl = nullptr;
  const void* m_window = nullptr;
  std::string m_glsl_version;

  std::filesystem::path m_current;

  std::filesystem::path m_baseFont;
  std::filesystem::path m_japanseseFont;
  std::filesystem::path m_iconFont;
  bool m_resetLayout = true;

  Gui();

public:
  void SetWindow(const void* window, const char* glsl_version);
  void Shutdown();
  ~Gui();
  Gui(const Gui&) = delete;
  Gui& operator=(const Gui&) = delete;
  static Gui& Instance()
  {
    static Gui s_instance;
    return s_instance;
  }

  std::vector<grapho::imgui::Dock> Docks;
  int FontSize = 20;
  void DarkMode();
  void AddDock(const grapho::imgui::Dock& dock)
  {
    bool visible = true;
    auto found = std::find_if(Docks.begin(), Docks.end(), [&dock](auto& d) {
      return d.Name == dock.Name;
    });
    if (found != Docks.end()) {
      visible = found->IsOpen;
      Docks.erase(found);
    }

    Docks.push_back(dock);
    Docks.back().IsOpen = visible;
  }
  void SetDockVisible(std::string_view name, bool visible)
  {
    for (auto& dock : Docks) {
      if (dock.Name == name) {
        dock.IsOpen = visible;
        return;
      }
    }

    Docks.push_back(
      { .Name = { name.begin(), name.end() }, .IsOpen = visible });
  }

  bool SetFont(const std::filesystem::path& path);
  bool AddJapaneseFont(const std::filesystem::path& path);
  bool AddIconFont(const std::filesystem::path& path);

  void LoadState(std::string_view ini);
  std::string SaveState();

  // return WantSaveIniSettings
  bool NewFrame();
  void DockSpace();
  void Render();

private:
  void Initialize();
};
