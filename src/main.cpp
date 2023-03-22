#include "camera.h"
#include "gl3renderer.h"
#include "gui.h"
#include "orbitview.h"
#include "platform.h"
#include "scene.h"

#include <imgui.h>

#include <ImGuizmo.h>

const auto WINDOW_WIDTH = 2000;
const auto WINDOW_HEIGHT = 1200;
const auto WINDOW_TITLE = "VrmEditor";

int main(int argc, char **argv) {
  Platform platform;
  auto window =
      platform.createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  if (!window) {
    return 1;
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

  float m[16]{
      1, 0, 0, 0, //
      0, 1, 0, 0, //
      0, 0, 1, 0, //
      0, 0, 0, 1, //
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
    ImGuizmo::BeginFrame();
    auto vp = ImGui::GetMainViewport();
    ImGuizmo::SetRect(vp->Pos.x, vp->Pos.y, vp->Size.x, vp->Size.y);
    ImGuizmo::DrawGrid(camera.view, camera.projection, m, 100);
    // ImGuizmo::DrawCubes(camera.view, camera.projection, m, 1);
    gui.update();
    gui.render();

    platform.present();
  }

  return 0;
}
