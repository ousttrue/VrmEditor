#pragma once
#include <DirectXMath.h>
#include <optional>
#include <stdint.h>
#include <string_view>

namespace libvrm {
namespace vrm {

enum class VrmVersion
{
  _0_x,
  _1_0,
};

enum class HumanBones
{
  unknown,
  hips,
  spine,
  chest,
  upperChest,
  neck,
  head,
  leftEye,
  rightEye,
  jaw,
  // arms
  leftShoulder,
  leftUpperArm,
  leftLowerArm,
  leftHand,
  rightShoulder,
  rightUpperArm,
  rightLowerArm,
  rightHand,
  // legs
  leftUpperLeg,
  leftLowerLeg,
  leftFoot,
  leftToes,
  rightUpperLeg,
  rightLowerLeg,
  rightFoot,
  rightToes,
  // fingers
  leftThumbMetacarpal,
  leftThumbProximal,
  leftThumbDistal,
  leftIndexProximal,
  leftIndexIntermediate,
  leftIndexDistal,
  leftMiddleProximal,
  leftMiddleIntermediate,
  leftMiddleDistal,
  leftRingProximal,
  leftRingIntermediate,
  leftRingDistal,
  leftLittleProximal,
  leftLittleIntermediate,
  leftLittleDistal,
  rightThumbMetacarpal,
  rightThumbProximal,
  rightThumbDistal,
  rightIndexProximal,
  rightIndexIntermediate,
  rightIndexDistal,
  rightMiddleProximal,
  rightMiddleIntermediate,
  rightMiddleDistal,
  rightRingProximal,
  rightRingIntermediate,
  rightRingDistal,
  rightLittleProximal,
  rightLittleIntermediate,
  rightLittleDistal,
  VRM_BONE_COUNT,
};
constexpr const char* HumanBonesNames[] = {
  "unknown",
  "hips",
  "spine",
  "chest",
  "upperChest",
  "neck",
  "head",
  "leftEye",
  "rightEye",
  "jaw",
  "leftShoulder",
  "leftUpperArm",
  "leftLowerArm",
  "leftHand",
  "rightShoulder",
  "rightUpperArm",
  "rightLowerArm",
  "rightHand",
  "leftUpperLeg",
  "leftLowerLeg",
  "leftFoot",
  "leftToes",
  "rightUpperLeg",
  "rightLowerLeg",
  "rightFoot",
  "rightToes",
  "leftThumbMetacarpal",
  "leftThumbProximal",
  "leftThumbDistal",
  "leftIndexProximal",
  "leftIndexIntermediate",
  "leftIndexDistal",
  "leftMiddleProximal",
  "leftMiddleIntermediate",
  "leftMiddleDistal",
  "leftRingProximal",
  "leftRingIntermediate",
  "leftRingDistal",
  "leftLittleProximal",
  "leftLittleIntermediate",
  "leftLittleDistal",
  "rightThumbMetacarpal",
  "rightThumbProximal",
  "rightThumbDistal",
  "rightIndexProximal",
  "rightIndexIntermediate",
  "rightIndexDistal",
  "rightMiddleProximal",
  "rightMiddleIntermediate",
  "rightMiddleDistal",
  "rightRingProximal",
  "rightRingIntermediate",
  "rightRingDistal",
  "rightLittleProximal",
  "rightLittleIntermediate",
  "rightLittleDistal",
};

inline std::optional<HumanBones>
HumanBoneFromName(std::string_view boneName, VrmVersion version)
{
  switch (version) {
    case VrmVersion::_0_x:
      if (boneName == "leftThumbProximal") {
        return HumanBones::leftThumbMetacarpal;
      } else if (boneName == "leftThumbIntermediate") {
        return HumanBones::leftThumbProximal;
      } else if (boneName == "rightThumbProximal") {
        return HumanBones::rightThumbMetacarpal;
      } else if (boneName == "rightThumbIntermediate") {
        return HumanBones::rightThumbProximal;
      }
      break;

    default:
      break;
  }

  for (int i = 0; i < std::size(HumanBonesNames); ++i) {
    if (boneName == HumanBonesNames[i]) {
      return (HumanBones)i;
    }
  }
  // not found
  return {};
}

inline const char*
HumanBoneToName(HumanBones bone)
{
  return HumanBonesNames[(int)bone];
}

inline DirectX::XMFLOAT4
HumanBoneToColor(HumanBones bone)
{
  switch (bone) {
    case HumanBones::leftThumbMetacarpal:
    case HumanBones::leftThumbProximal:
    case HumanBones::leftThumbDistal:
    case HumanBones::leftIndexProximal:
    case HumanBones::leftIndexIntermediate:
    case HumanBones::leftIndexDistal:
    case HumanBones::leftMiddleProximal:
    case HumanBones::leftMiddleIntermediate:
    case HumanBones::leftMiddleDistal:
    case HumanBones::leftRingProximal:
    case HumanBones::leftRingIntermediate:
    case HumanBones::leftRingDistal:
    case HumanBones::leftLittleProximal:
    case HumanBones::leftLittleIntermediate:
    case HumanBones::leftLittleDistal:
    case HumanBones::rightThumbMetacarpal:
    case HumanBones::rightThumbProximal:
    case HumanBones::rightThumbDistal:
    case HumanBones::rightIndexProximal:
    case HumanBones::rightIndexIntermediate:
    case HumanBones::rightIndexDistal:
    case HumanBones::rightMiddleProximal:
    case HumanBones::rightMiddleIntermediate:
    case HumanBones::rightMiddleDistal:
    case HumanBones::rightRingProximal:
    case HumanBones::rightRingIntermediate:
    case HumanBones::rightRingDistal:
    case HumanBones::rightLittleProximal:
    case HumanBones::rightLittleIntermediate:
    case HumanBones::rightLittleDistal:
      return { 0.5f, 0.5f, 0.4f, 1 };

    default:
      return { 0.5f, 1, 0.8f, 1 };
  }
}

}
} // namespace vrm
