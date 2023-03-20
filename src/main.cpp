#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <stdio.h>
#include <tuple>

#include "camera.h"
#include "gl3renderer.h"
#include "gui.h"
#include "orbitview.h"
#include "scene.h"

const auto WINDOW_WIDTH = 2000;
const auto WINDOW_HEIGHT = 1200;
const auto WINDOW_TITLE = "VrmEditor";

static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}

struct WindowSize {
  int width;
  int height;
};
class Platform {
  GLFWwindow *m_window = nullptr;

public:
  std::string glsl_version;

  Platform() {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
      throw std::runtime_error("glfwInit");
    }
  }

  ~Platform() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
  }

  GLFWwindow *createWindow() {
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
    // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
#endif

    m_window =
        glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (!m_window) {
      return nullptr;
    }
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);
    return m_window;
  }

  std::optional<WindowSize> newFrame() {
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

    return WindowSize{width, height};
  }

  void present() { glfwSwapBuffers(m_window); }
};

int main(int argc, char **argv) {
  Platform platform;
  auto window = platform.createWindow();
  if (!window) {
    return -1;
  }

  Gl3Renderer gl3r;
  Gui gui(window, platform.glsl_version.c_str());
  Camera camera{};
  OrbitView view;
  Scene scene;

  if (argc > 1) {
    scene.load(argv[1]);
  }

  RenderFunc render = [&gl3r](const Camera &camera, const Mesh &mesh,
                              const float m[16]) {
    gl3r.render(camera, mesh, m);
  };

  while (auto size = platform.newFrame()) {
    // newFrame
    gui.newFrame();
    camera.resize(size->width, size->height);
    view.SetSize(size->width, size->height);
    if (auto event = gui.backgroundMouseEvent()) {
      if (auto delta = event->rightDrag) {
        view.YawPitch(delta->x, delta->y);
      }
      if (auto delta = event->middleDrag) {
        view.Shift(delta->x, delta->y);
      }
      if (auto wheel = event->wheel) {
        view.Dolly(*wheel);
      }
    }
    view.Update(camera.projection, camera.view);

    // render view
    gl3r.clear(camera);
    scene.render(camera, render);

    // render gui
    gui.render();

    platform.present();
  }
  return 0;
}
