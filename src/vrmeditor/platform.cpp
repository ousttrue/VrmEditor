#include "platform.h"
#include <plog/Log.h>
#include <stdexcept>
#include <vrm/timeline.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Windows.h>
#endif

int Platform::Width = 2000;
int Platform::Height = 1200;
bool Platform::IsMaximized = false;

static void
error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}

Platform::Platform()
{
  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
    throw std::runtime_error("glfwInit");
  }
}

Platform::~Platform()
{
  glfwDestroyWindow(m_window);
  glfwTerminate();
}

GLFWwindow*
Platform::WindowCreate(const char* title)
{
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  // GL 3.2 + GLSL 150
  glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
  // GL 3.0 + GLSL 130
  glsl_version = "#version 130";
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
  // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

  if (Width == 0) {
    Width = 640;
  }
  if (Height == 0) {
    Height = 480;
  }
  m_window = glfwCreateWindow(Width, Height, title, NULL, NULL);
  if (!m_window) {
    return nullptr;
  }

#ifdef _WIN32
  auto hWnd = glfwGetWin32Window(m_window);
  HICON hIcon = LoadIcon(GetModuleHandle(nullptr), "APPICON");
  SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
  SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
#endif

  glfwSetWindowUserPointer(m_window, this);

  if (IsMaximized) {
    glfwMaximizeWindow(m_window);
  }
  glfwMakeContextCurrent(m_window);
  glfwSwapInterval(1);

  auto drop_callback = +[](GLFWwindow* window, int count, const char** paths) {
    auto self = (Platform*)glfwGetWindowUserPointer(window);
    for (int i = 0; i < count; i++) {
      std::filesystem::path drop = paths[i];
      PLOG_INFO << "drop_callback[" << i << "]: " << drop.string();
      for (auto& callback : self->OnDrops) {
        callback(drop);
      }
    }
  };
  glfwSetDropCallback(m_window, drop_callback);

  return m_window;
}

std::optional<FrameInfo>
Platform::NewFrame()
{
  if (glfwWindowShouldClose(m_window)) {
    return {};
  }
  // Poll and handle events (inputs, window resize, etc.)
  // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
  // tell if dear imgui wants to use your inputs.
  // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
  // your main application, or clear/overwrite your copy of the mouse data.
  // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
  // data to your main application, or clear/overwrite your copy of the
  // keyboard data. Generally you may always pass all inputs to dear imgui,
  // and hide them from your application based on those two flags.
  glfwPollEvents();

  int width, height;
  glfwGetFramebufferSize(m_window, &width, &height);

  auto seconds = glfwGetTime();

  return FrameInfo{
    .Width = width,
    .Height = height,
    .Time = libvrm::Time(seconds),
  };
}

void
Platform::Present()
{
  glfwSwapBuffers(m_window);
}

void
Platform::SetTitle(const std::string& title)
{
  glfwSetWindowTitle(m_window, title.c_str());
}

std::tuple<int, int>
Platform::WindowSize() const
{
  int width, height;
  glfwGetWindowSize(m_window, &width, &height);
  return { width, height };
}

bool
Platform::IsWindowMaximized() const
{
  return glfwGetWindowAttrib(m_window, GLFW_MAXIMIZED);
}

void
Platform::SetWindowSize(int width, int height, bool maximize)
{
  Width = width;
  Height = height;
  IsMaximized = maximize;
  if (m_window) {
    glfwSetWindowSize(m_window, width, height);
    if (maximize) {
      glfwMaximizeWindow(m_window);
    }
  }
}

void
Platform::CopyText(const std::string& text)
{
  glfwSetClipboardString(m_window, text.c_str());
}

std::string
Platform::PasteText()
{
  return glfwGetClipboardString(m_window);
}
