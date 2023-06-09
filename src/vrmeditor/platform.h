#pragma once
#include <chrono>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vrm/timeline.h>

struct FrameInfo
{
  int Width;
  int Height;
  libvrm::Time Time;
};

using OnDropFunc = std::function<void(const std::filesystem::path&)>;

class Platform
{
  struct GLFWwindow* m_window = nullptr;

  Platform();

public:
  static int Width;
  static int Height;
  static bool IsMaximized;

  std::list<OnDropFunc> OnDrops;
  std::string glsl_version;
  ~Platform();
  Platform(const Platform&) = delete;
  Platform& operator=(const Platform&) = delete;
  static Platform& Instance()
  {
    static Platform s_instance;
    return s_instance;
  }
  GLFWwindow* WindowCreate(const char* title);
  std::optional<FrameInfo> NewFrame();
  void Present();
  void SetTitle(const std::string& title);
  std::tuple<int, int> WindowSize() const;
  bool IsWindowMaximized() const;
  void SetWindowSize(int width, int height, bool maximize);
};
