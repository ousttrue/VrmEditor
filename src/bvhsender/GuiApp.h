#pragma once
#include <grapho/camera/camera.h>

class GuiApp
{
  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;
  float clear_color_[4] = { 0.45f, 0.55f, 0.60f, 1.00f };

public:
  grapho::camera::Camera camera;
  float clear_color[4] = {};
  GuiApp(const GuiApp&) = delete;
  GuiApp& operator=(const GuiApp&) = delete;
  GuiApp();
  ~GuiApp();
  void UpdateGui();
  struct ImDrawData* RenderGui();

private:
  void UpdateGuiDockspace();
};
