#pragma once
#include <DirectXMath.h>
#include <optional>
#include <stdint.h>
#include <string_view>

namespace libvrm {

enum class VrmVersion
{
  _0_x,
  _1_0,
};

enum class HumanBones
{
  hips = 1,
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
  tip,
  // VRM_BONE_COUNT = tip,
};

struct HumanBonesRange
{
  class Iterator
  {
    int m_value;

  public:
    Iterator(HumanBones value)
      : m_value((int)value)
    {
    }
    HumanBones operator*(void) const { return (HumanBones)m_value; }
    void operator++(void) { ++m_value; }
    bool operator!=(Iterator rhs) { return m_value != rhs.m_value; }
  };

  struct Range
  {
    HumanBones m_begin;
    HumanBones m_end;
    Iterator begin() const { return { m_begin }; }
    Iterator end() const { return { m_end }; }
  };
  constexpr static Range All{ HumanBones::hips, HumanBones::tip };
  constexpr static Range Body{ HumanBones::hips,
                               HumanBones::leftThumbMetacarpal };
  constexpr static Range LeftFingers{ HumanBones::leftThumbMetacarpal,
                                      HumanBones::rightThumbMetacarpal };
  constexpr static Range RightFingers{ HumanBones::rightThumbMetacarpal,
                                       HumanBones::tip };
};

inline bool
HumanBoneIsFinger(HumanBones bone)
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
      return true;

    default:
      return false;
  }
}
constexpr const char* HumanBonesNames[] = {
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
constexpr const char* HumanBonesNamesWithIcon[] = {
  "🦴hips",
  "🦴spine",
  "🦴chest",
  "🦴upperChest",
  "🦴neck",
  "🦴head",
  "👀leftEye",
  "👀rightEye",
  "🐲jaw",
  "💪leftShoulder",
  "💪leftUpperArm",
  "💪leftLowerArm",
  "💪leftHand",
  "💪rightShoulder",
  "💪rightUpperArm",
  "💪rightLowerArm",
  "💪rightHand",
  "🦵leftUpperLeg",
  "🦵leftLowerLeg",
  "🦵leftFoot",
  "🦵leftToes",
  "🦵rightUpperLeg",
  "🦵rightLowerLeg",
  "🦵rightFoot",
  "🦵rightToes",
  "🖐leftThumbMetacarpal",
  "🖐leftThumbProximal",
  "🖐leftThumbDistal",
  "🖐leftIndexProximal",
  "🖐leftIndexIntermediate",
  "🖐leftIndexDistal",
  "🖐leftMiddleProximal",
  "🖐leftMiddleIntermediate",
  "🖐leftMiddleDistal",
  "🖐leftRingProximal",
  "🖐leftRingIntermediate",
  "🖐leftRingDistal",
  "🖐leftLittleProximal",
  "🖐leftLittleIntermediate",
  "🖐leftLittleDistal",
  "🖐rightThumbMetacarpal",
  "🖐rightThumbProximal",
  "🖐rightThumbDistal",
  "🖐rightIndexProximal",
  "🖐rightIndexIntermediate",
  "🖐rightIndexDistal",
  "🖐rightMiddleProximal",
  "🖐rightMiddleIntermediate",
  "🖐rightMiddleDistal",
  "🖐rightRingProximal",
  "🖐rightRingIntermediate",
  "🖐rightRingDistal",
  "🖐rightLittleProximal",
  "🖐rightLittleIntermediate",
  "🖐rightLittleDistal",
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
inline const char*
HumanBoneToNameWithIcon(HumanBones bone)
{
  return HumanBonesNamesWithIcon[(int)bone];
}

inline DirectX::XMFLOAT4
HumanBoneToColor(HumanBones bone)
{
  switch (bone) {
    // arms
    case HumanBones::leftShoulder:
    case HumanBones::leftUpperArm:
    case HumanBones::leftLowerArm:
    case HumanBones::rightShoulder:
    case HumanBones::rightUpperArm:
    case HumanBones::rightLowerArm:
    case HumanBones::leftUpperLeg:
    case HumanBones::leftLowerLeg:
    case HumanBones::leftFoot:
    case HumanBones::leftToes:
    case HumanBones::rightUpperLeg:
    case HumanBones::rightLowerLeg:
    case HumanBones::rightFoot:
    case HumanBones::rightToes:
    case HumanBones::leftHand:
    case HumanBones::rightHand:
      return { 1, 0.8f, 0.5f, 1 };

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

inline DirectX::XMFLOAT2
HumanBoneToWidthDepth(HumanBones bone)
{
  switch (bone) {
    case HumanBones::spine:
      return { 0.15f, 0.10f };

    case HumanBones::hips:
    case HumanBones::chest:
    case HumanBones::upperChest:
      return { 0.20f, 0.12f };

    case HumanBones::neck:
      return { 0.06f, 0.06f };

    // case HumanBones::head:
    //   return { 0.2f, 0.2f };

    // case HumanBones::leftEye:
    // case HumanBones::rightEye:
    // case HumanBones::jaw:
    // arms
    // case HumanBones::leftShoulder:
    // case HumanBones::leftUpperArm:
    // case HumanBones::leftLowerArm:
    // case HumanBones::rightShoulder:
    // case HumanBones::rightUpperArm:
    // case HumanBones::rightLowerArm:
    case HumanBones::leftHand:
    case HumanBones::rightHand:
      return { 0.01f, 0.05f };
    // legs
    // case HumanBones::leftUpperLeg:
    // case HumanBones::leftLowerLeg:
    // case HumanBones::leftFoot:
    // case HumanBones::leftToes:
    // case HumanBones::rightUpperLeg:
    // case HumanBones::rightLowerLeg:
    // case HumanBones::rightFoot:
    // case HumanBones::rightToes:
    // fingers
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
      return { 0.01f, 0.01f };
      // case HumanBones::tip:

    default:
      return { 0.05f, 0.05f };
  }
}

} // namespace
