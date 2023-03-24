#pragma once
#include <functional>
#include <list>
#include <memory>
#include <optional>

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

class Dock {
  DockShow on_show;

public:
  bool popen = true;
  Dock(const DockShow &on_show) : on_show(on_show) {}
  void show() {
    if (popen) {
      on_show(&popen);
    }
  }
};

class Gui {

public:
  std::list<Dock> m_docks;
  Gui(const void *window, const char *glsl_version);
  ~Gui();
  std::optional<MouseEvent> backgroundMouseEvent() const;
  void newFrame();
  void update();
  void render();

private:
  void dockspace();
};
