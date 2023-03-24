#pragma once
#include <functional>
#include <list>
#include <memory>
#include <optional>
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
