#pragma once
#include <chrono>
#include <optional>
#include <string>
#include <vrm/timeline.h>

struct FrameInfo {
  int width;
  int height;
  Time time;
};
class Platform {
  struct GLFWwindow *m_window = nullptr;

public:
  std::string glsl_version;
  Platform();
  ~Platform();
  GLFWwindow *createWindow(int width, int height, const char *title);
  std::optional<FrameInfo> newFrame();
  void present();
};
