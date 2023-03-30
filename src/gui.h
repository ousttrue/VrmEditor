#pragma once
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <string_view>

struct Delta {
  int X;
  int Y;
};
struct MouseEvent {
  std::optional<Delta> RightDrag;
  std::optional<Delta> MiddleDrag;
  std::optional<int> Wheel;
};

using DockShow = std::function<void(bool *popen)>;

struct Dock {
  std::string Name;
  DockShow OnShow;
  bool UseWindow = true;
  bool IsOpen = true;

  Dock(std::string_view name, const DockShow &show)
      : Name(name), OnShow(show), UseWindow(false) {}
  Dock(std::string_view name, const std::function<void()> &show)
      : Name(name), UseWindow(true) {
    OnShow = [show](bool *) { show(); };
  }

  void Show();
};

struct FontConfig {
  // use default if empty
  std::string Font;
};

using Task = std::function<void()>;

class Gui {
  bool m_initialized = false;
  const void *m_window = nullptr;

  std::string m_glsl_version;
  std::queue<Task> m_tasks;

  int m_fontSize = 20;
  std::vector<FontConfig> m_fonts = {{}};

public:
  std::list<Dock> m_docks;
  Gui(const void *window, const char *glsl_version);
  ~Gui();
  std::optional<MouseEvent> BackgroundMouseEvent() const;
  void NewFrame();
  void DockSpace();
  void Render();
  void PostTask(const Task &task) { m_tasks.push(task); }
  void LoadFont();
};
