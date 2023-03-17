#pragma once

class Gui {
  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;

public:
  Gui(const void *window, const char *glsl_version);
  ~Gui();
  void render();
};
