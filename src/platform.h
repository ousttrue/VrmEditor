#pragma once
#include <optional>
#include <string>

struct WindowSize {
  int width;
  int height;
};
class Platform {
  struct GLFWwindow *m_window = nullptr;

public:
  std::string glsl_version;
  Platform();
  ~Platform();
  GLFWwindow *createWindow(int width, int height, const char *title);
  std::optional<WindowSize> newFrame();
  void present();
};
