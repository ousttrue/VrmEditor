#pragma once
#include "gl3renderer.h"
#include "gui.h"
#include "orbitview.h"
#include "rendertarget.h"
#include <Bvh.h>
#include <BvhSolver.h>
#include <cuber/gl3/GlCubeRenderer.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <filesystem>
#include <functional>
#include <imgui.h>
#include <imnodes.h>
#include <list>
#include <vrm/humanbones.h>
#include <vrm/humanpose.h>
#include <vrm/timeline.h>

using OnPose = std::function<void(const vrm::HumanPose&)>;

struct MotionSource
{
  std::shared_ptr<Bvh> Motion;
  std::shared_ptr<BvhSolver> MotionSolver;
  std::vector<vrm::HumanBones> HumanBoneMap = {
    vrm::HumanBones::hips,          vrm::HumanBones::spine,
    vrm::HumanBones::chest,         vrm::HumanBones::neck,
    vrm::HumanBones::head,          vrm::HumanBones::leftShoulder,
    vrm::HumanBones::leftUpperArm,  vrm::HumanBones::leftLowerArm,
    vrm::HumanBones::leftHand,      vrm::HumanBones::rightShoulder,
    vrm::HumanBones::rightUpperArm, vrm::HumanBones::rightLowerArm,
    vrm::HumanBones::rightHand,     vrm::HumanBones::leftUpperLeg,
    vrm::HumanBones::leftLowerLeg,  vrm::HumanBones::leftFoot,
    vrm::HumanBones::leftToe,       vrm::HumanBones::rightUpperLeg,
    vrm::HumanBones::rightLowerLeg, vrm::HumanBones::rightFoot,
    vrm::HumanBones::rightToe,
  };
  std::list<OnPose> PoseCallbacks;

  void SetHumanPose(const vrm::HumanPose& pose)
  {
    for (auto& callback : PoseCallbacks) {
      callback(pose);
    }
  }

  bool LoadMotion(const std::filesystem::path& path,
                  float scaling,
                  const std::shared_ptr<Timeline>& timeline)
  {
    Motion = Bvh::ParseFile(path);
    if (!Motion) {
      return false;
    }

    MotionSolver = std::make_shared<BvhSolver>();
    MotionSolver->Initialize(Motion);

    auto track = timeline->AddTrack("bvh", Motion->Duration());
    track->Callbacks.push_back([this](auto time, bool repeat) {
      auto index = Motion->TimeToIndex(time);
      auto frame = Motion->GetFrame(index);
      MotionSolver->ResolveFrame(frame);

      // human pose to scene
      auto& hips = MotionSolver->instances_[0];
      SetHumanPose({ .RootPosition = { hips._41, hips._42, hips._43 },
                     .Bones = HumanBoneMap,
                     .Rotations = MotionSolver->localRotations });
    });
    return true;
  }
};

class MotionDock
{
public:
  static void Create(const AddDockFunc& addDock,
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
};
