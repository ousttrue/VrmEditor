#include "camera.h"
#include "gl3renderer.h"
#include "gui.h"
#include "no_sal2.h"
#include "orbitview.h"
#include "platform.h"
#include <vrm/scene.h>

#include <imgui.h>

#include <ImGuizmo.h>
#include <imgui_neo_sequencer.h>

const auto WINDOW_WIDTH = 2000;
const auto WINDOW_HEIGHT = 1200;
const auto WINDOW_TITLE = "VrmEditor";

struct TreeContext {
  Node *selected = nullptr;
  Node *new_selected = nullptr;
};

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

  TreeContext context;

  int32_t currentFrame = 0;
  int32_t startFrame = -10;
  int32_t endFrame = 64;
  static bool transformOpen = false;

  while (auto info = platform.newFrame()) {
    // newFrame
    gui.newFrame();
    camera.resize(info->width, info->height);
    view.SetSize(info->width, info->height);
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
    scene.render(camera, render, info->time);

    // render gui
    ImGuizmo::BeginFrame();
    auto vp = ImGui::GetMainViewport();
    ImGuizmo::SetRect(vp->Pos.x, vp->Pos.y, vp->Size.x, vp->Size.y);
    ImGuizmo::DrawGrid(camera.view, camera.projection, m, 100);
    // ImGuizmo::DrawCubes(camera.view, camera.projection, m, 1);

    context.selected = context.new_selected;
    auto enter = [&context, &camera](Node &node,
                                     const DirectX::XMFLOAT4X4 &parent) {
      ImGui::SetNextItemOpen(true, ImGuiCond_Once);
      static ImGuiTreeNodeFlags base_flags =
          ImGuiTreeNodeFlags_OpenOnArrow |
          ImGuiTreeNodeFlags_OpenOnDoubleClick |
          ImGuiTreeNodeFlags_SpanAvailWidth;
      ImGuiTreeNodeFlags node_flags = base_flags;
      auto is_selected = context.selected == &node;
      if (is_selected) {
        node_flags |= ImGuiTreeNodeFlags_Selected;

        auto m = node.world;

        if (ImGuizmo::Manipulate(camera.view, camera.projection,
                                 ImGuizmo::UNIVERSAL, ImGuizmo::LOCAL,
                                 (float *)&m, NULL, NULL, NULL, NULL)) {
          // decompose feedback
          node.setWorldMatrix(m, parent);
        }
      }

      if (node.children.empty()) {
        node_flags |=
            ImGuiTreeNodeFlags_Leaf |
            ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
      }

      bool node_open = ImGui::TreeNodeEx((void *)(intptr_t)node.index,
                                         node_flags, "%s", node.name.c_str());
      if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        context.new_selected = &node;
      }
      return node.children.size() && node_open;
    };
    auto leave = [](Node &) { ImGui::TreePop(); };
    ImGui::Begin("scene");
    scene.traverse(enter, leave);
    ImGui::End();

    ImGui::Begin("timeline");

    if (ImGui::BeginNeoSequencer("Sequencer", &currentFrame, &startFrame,
                                 &endFrame)) {
      if (ImGui::BeginNeoGroup("Transform", &transformOpen)) {
        std::vector<ImGui::FrameIndexType> keys = {0, 10, 24};
        if (ImGui::BeginNeoTimeline("Position", keys)) {
          ImGui::EndNeoTimeLine();
        }
        ImGui::EndNeoGroup();
      }

      // Timeline code here
      ImGui::EndNeoSequencer();
    }

    ImGui::End();

    gui.update();
    gui.render();

    platform.present();
  }

  return 0;
}
