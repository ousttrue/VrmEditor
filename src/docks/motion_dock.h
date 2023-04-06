#pragma once
#include "gui.h"
#include <filesystem>
#include <functional>
#include <list>
#include <memory>
#include <vrm/bvh.h>
#include <vrm/bvhsolver.h>
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

struct GraphPin
{
  std::string Name;
};

struct GraphNode
{
  std::string Prefix;
  std::string Name;
  std::vector<GraphPin> Outputs;
  std::vector<GraphPin> Inputs;
};

class MotionDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     const std::shared_ptr<MotionSource>& motion_source);
};
