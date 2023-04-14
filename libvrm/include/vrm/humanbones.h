#pragma once
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

}
} // namespace vrm
