#pragma once
#include <filesystem>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <string_view>

struct Delta
{
  int X;
  int Y;
};

struct MouseEvent
{
  std::optional<Delta> RightDrag;
  std::optional<Delta> MiddleDrag;
  std::optional<int> Wheel;
};

using DockShow = std::function<void(const char* title, bool* popen)>;

struct Dock
{
  std::string Name;
  DockShow OnShow;
  bool UseWindow = true;
  bool IsOpen = true;

  Dock(std::string_view name, const DockShow& show)
    : Name(name)
    , OnShow(show)
    , UseWindow(false)
  {
  }
  Dock(std::string_view name, const std::function<void()>& show)
    : Name(name)
    , UseWindow(true)
  {
    OnShow = [show](const char* title, bool*) { show(); };
  }

  void Show();
};

using AddDockFunc = std::function<void(const Dock& dock)>;

using Task = std::function<void()>;

class Gui
{
  std::filesystem::path m_current;

  bool m_initialized = false;
  const void* m_window = nullptr;

  std::string m_glsl_version;
  std::queue<Task> m_tasks;

  std::filesystem::path m_baseFont;
  std::filesystem::path m_japanseseFont;
  std::filesystem::path m_iconFont;
  bool m_resetLayout = false;

public:
  std::list<Dock> Docks;
  int FontSize = 20;
  Gui(const void* window, const char* glsl_version);
  ~Gui();
  void AddDock(const Dock& dock)
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

    Docks.push_back(Dock(name, []() {}));
    Docks.back().IsOpen = visible;
  }

  void LoadState(std::string_view ini);
  std::string SaveState();

  std::optional<MouseEvent> BackgroundMouseEvent() const;
  // return WantSaveIniSettings
  bool NewFrame();
  void DockSpace();
  void Render();
  void PostTask(const Task& task) { m_tasks.push(task); }

  void LoadFont();
  bool SetFont(const std::filesystem::path& path);
  bool AddJapaneseFont(const std::filesystem::path& path);
  bool AddIconFont(const std::filesystem::path& path);
};
