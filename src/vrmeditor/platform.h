#pragma once
#include <chrono>
#include <optional>
#include <string>
#include <vrm/timeline.h>

struct FrameInfo {
  int Width;
  int Height;
  libvrm::Time Time;
};

class Platform {
  struct GLFWwindow *m_window = nullptr;

public:
  std::string glsl_version;
  Platform();
  ~Platform();
  GLFWwindow *CreateWindow(int width, int height, const char *title);
  std::optional<FrameInfo> NewFrame();
  void Present();
  void SetTitle(const std::string &title);
};
