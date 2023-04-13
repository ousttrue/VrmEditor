#pragma once
#include <grapho/orbitview.h>

class GuiApp
{
  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;
  float clear_color_[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
  grapho::OrbitView turntable_;

public:
  float clear_color[4] = {};
  float projection[16] = {
    1, 0, 0, 0, //
    0, 1, 0, 0, //
    0, 0, 1, 0, //
    0, 0, 0, 1, //
  };
  float view[16] = {
    1, 0, 0, 0, //
    0, 1, 0, 0, //
    0, 0, 1, 0, //
    0, 0, 0, 1, //
  };
  GuiApp(const GuiApp&) = delete;
  GuiApp& operator=(const GuiApp&) = delete;
  GuiApp();
  ~GuiApp();
  void UpdateGui();
  struct ImDrawData* RenderGui();

private:
  void UpdateGuiDockspace();
};
