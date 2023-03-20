#pragma once
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

class Gui {
  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;

public:
  Gui(const void *window, const char *glsl_version);
  ~Gui();
  std::optional<MouseEvent> backgroundMouseEvent() const;
  void newFrame();
  void render();
};
