#pragma once
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <queue>
#include <string>

struct Delta {
  int x;
  int y;
};
struct MouseEvent {
  std::optional<Delta> rightDrag;
  std::optional<Delta> middleDrag;
  std::optional<int> wheel;
};

using DockShow = std::function<void(bool *popen)>;

struct Dock {
  std::string name;
  DockShow on_show;
  bool use_window = true;
  bool p_open = true;

  Dock(std::string_view name, const DockShow &show)
      : name(name), on_show(show), use_window(false) {}
  Dock(std::string_view name, const std::function<void()> &show)
      : name(name), on_show(on_show), use_window(true) {
    on_show = [show](bool *) { show(); };
  }

  void show();
};

struct FontConfig {
  // use default if empty
  std::string font;
};

using Task = std::function<void()>;

class Gui {
  bool initialized_ = false;
  const void *window_ = nullptr;

  std::string glsl_version_;
  std::queue<Task> tasks_;

  int fontSize_ = 20;
  std::vector<FontConfig> fonts_ = {{}};

public:
  std::list<Dock> m_docks;
  Gui(const void *window, const char *glsl_version);
  ~Gui();
  std::optional<MouseEvent> backgroundMouseEvent() const;
  void newFrame();
  void update();
  void render();
  void postTask(const Task &task) { tasks_.push(task); }
  void loadFont();

private:
  void dockspace();
};
