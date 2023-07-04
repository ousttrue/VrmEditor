#pragma once
#include "humanbones.h"
#include <DirectXMath.h>
#include <span>
#include <vector>

namespace libvrm {

struct HumanPose
{
  DirectX::XMFLOAT3 RootPosition = { 0, 0, 0 };
  // require external payload
  std::span<const HumanBones> Bones;
  std::span<const DirectX::XMFLOAT4> Rotations;

  static HumanPose Initial()
  {
    static const HumanBones s_bones[] = {
      // 6
      HumanBones::hips,
      HumanBones::spine,
      HumanBones::chest,
      HumanBones::upperChest,
      HumanBones::neck,
      HumanBones::head,
      // HumanBones::leftEye,
      // HumanBones::rightEye,
      // HumanBones::jaw,
      // arms 8
      HumanBones::leftShoulder,
      HumanBones::leftUpperArm,
      HumanBones::leftLowerArm,
      HumanBones::leftHand,
      HumanBones::rightShoulder,
      HumanBones::rightUpperArm,
      HumanBones::rightLowerArm,
      HumanBones::rightHand,
      // legs 8
      HumanBones::leftUpperLeg,
      HumanBones::leftLowerLeg,
      HumanBones::leftFoot,
      HumanBones::leftToes,
      HumanBones::rightUpperLeg,
      HumanBones::rightLowerLeg,
      HumanBones::rightFoot,
      HumanBones::rightToes,
      // fingers 30
      HumanBones::leftThumbMetacarpal,
      HumanBones::leftThumbProximal,
      HumanBones::leftThumbDistal,
      HumanBones::leftIndexProximal,
      HumanBones::leftIndexIntermediate,
      HumanBones::leftIndexDistal,
      HumanBones::leftMiddleProximal,
      HumanBones::leftMiddleIntermediate,
      HumanBones::leftMiddleDistal,
      HumanBones::leftRingProximal,
      HumanBones::leftRingIntermediate,
      HumanBones::leftRingDistal,
      HumanBones::leftLittleProximal,
      HumanBones::leftLittleIntermediate,
      HumanBones::leftLittleDistal,
      HumanBones::rightThumbMetacarpal,
      HumanBones::rightThumbProximal,
      HumanBones::rightThumbDistal,
      HumanBones::rightIndexProximal,
      HumanBones::rightIndexIntermediate,
      HumanBones::rightIndexDistal,
      HumanBones::rightMiddleProximal,
      HumanBones::rightMiddleIntermediate,
      HumanBones::rightMiddleDistal,
      HumanBones::rightRingProximal,
      HumanBones::rightRingIntermediate,
      HumanBones::rightRingDistal,
      HumanBones::rightLittleProximal,
      HumanBones::rightLittleIntermediate,
      HumanBones::rightLittleDistal,
    };
    static const DirectX::XMFLOAT4 s_rotations[6 + 8 + 8 + 30] = {
      { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 },
      { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 },
      { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 },
      { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 },
      { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 },
      { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 },
      { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 },
      { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 },
      { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 },
      { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 },
      { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 },
      { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 },
      { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 },
    };
    assert(std::size(s_bones) == std::size(s_rotations));
    return { { 0, 0, 0 }, s_bones, s_rotations };
  }
};

struct HumanPosePayload
{
  DirectX::XMFLOAT3 RootPosition = { 0, 0, 0 };
  std::vector<HumanBones> Bones;
  std::vector<DirectX::XMFLOAT4> Rotations;

  void SetPose(const HumanPose& pose)
  {
    RootPosition = pose.RootPosition;
    Bones.assign(pose.Bones.begin(), pose.Bones.end());
    Rotations.assign(pose.Rotations.begin(), pose.Rotations.end());
  }

  HumanPose GetPose() const
  {
    return {
      RootPosition,
      Bones,
      Rotations,
    };
  }
};

} // namespace
