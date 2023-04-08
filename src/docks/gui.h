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
  std::u8string m_imgui_ini;
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
  int m_fontSize = 20;
  std::list<Dock> m_docks;
  Gui(const void* window, const char* glsl_version);
  ~Gui();
  std::optional<MouseEvent> BackgroundMouseEvent() const;
  void NewFrame();
  void DockSpace();
  void Render();
  void PostTask(const Task& task) { m_tasks.push(task); }

  void LoadFont();
  void SetFontSize(int size) { m_fontSize = size; }
  bool SetFont(const std::filesystem::path& path);
  bool AddJapaneseFont(const std::filesystem::path& path);
  bool AddIconFont(const std::filesystem::path& path);
};
