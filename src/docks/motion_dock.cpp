#include <GL/glew.h>

#include "motion_dock.h"
#include "gl3renderer.h"
#include "gui.h"
#include "orbitview.h"
#include "rendertarget.h"
#include <Bvh.h>
#include <BvhSolver.h>
#include <cuber/gl3/GlCubeRenderer.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <imgui.h>
#include <imnodes.h>
#include <vrm/humanbones.h>
#include <vrm/humanpose.h>

void
MotionDock::Create(const AddDockFunc& addDock,
                   const std::shared_ptr<MotionSource>& motion_source)
{
  auto rt = std::make_shared<RenderTarget>(std::make_shared<OrbitView>());
  rt->color[0] = 0.4f;
  rt->color[1] = 0.2f;
  rt->color[2] = 0.2f;
  rt->color[3] = 1.0f;

  auto cuber = std::make_shared<cuber::gl3::GlCubeRenderer>();
  auto liner = std::make_shared<cuber::gl3::GlLineRenderer>();
  std::vector<grapho::LineVertex> lines;
  cuber::PushGrid(lines);

  rt->render =
    [cuber, liner, lines, motion_source](const ViewProjection& camera) {
      if (motion_source->MotionSolver) {
        cuber->Render(camera.projection,
                      camera.view,
                      motion_source->MotionSolver->instances_.data(),
                      motion_source->MotionSolver->instances_.size());
      }
      liner->Render(camera.projection, camera.view, lines);
    };

  auto gl3r = std::make_shared<Gl3Renderer>();

  addDock(Dock("motion", [rt](bool* p_open) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
    if (ImGui::Begin("motion",
                     p_open,
                     ImGuiWindowFlags_NoScrollbar |
                       ImGuiWindowFlags_NoScrollWithMouse)) {
      auto pos = ImGui::GetWindowPos();
      pos.y += ImGui::GetFrameHeight();
      auto size = ImGui::GetContentRegionAvail();
      rt->show_fbo(pos.x, pos.y, size.x, size.y);
    }
    ImGui::End();
    ImGui::PopStyleVar();
  }));

  addDock(Dock("input-stream", []() {
    const int hardcoded_node_id = 1;

    ImNodes::BeginNodeEditor();

    ImNodes::BeginNode(hardcoded_node_id);
    ImGui::Dummy(ImVec2(80.0f, 45.0f));
    ImNodes::EndNode();

    ImNodes::EndNodeEditor();
  }));
}
