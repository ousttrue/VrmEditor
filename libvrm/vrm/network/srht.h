#pragma once
#include "../humanoid/humanbones.h"
#include "quat_packer.h"
#include <stdint.h>

//
// SingleRootHierarchicalTransformation
//
namespace libvrm::srht {

enum class HumanoidBones : uint16_t
{
  // body: 6
  HIPS = 0,
  SPINE,
  CHEST,
  UPPER_CHEST,
  NECK,
  HEAD,
  // legs: 4 x 2
  LEFT_UPPER_LEG,
  LEFT_LOWER_LEG,
  LEFT_FOOT,
  LEFT_TOES,
  RIGHT_UPPER_LEG,
  RIGHT_LOWER_LEG,
  RIGHT_FOOT,
  RIGHT_TOES,
  // arms: 4 x 2
  LEFT_SHOULDER,
  LEFT_UPPER_ARM,
  LEFT_LOWER_ARM,
  LEFT_HAND,
  RIGHT_SHOULDER,
  RIGHT_UPPER_ARM,
  RIGHT_LOWER_ARM,
  RIGHT_HAND,
  // fingers: 3 x 5 x 2
  LEFT_THUMB_METACARPAL,
  LEFT_THUMB_PROXIMAL,
  LEFT_THUMB_DISTAL,
  LEFT_INDEX_PROXIMAL,
  LEFT_INDEX_INTERMEDIATE,
  LEFT_INDEX_DISTAL,
  LEFT_MIDDLE_PROXIMAL,
  LEFT_MIDDLE_INTERMEDIATE,
  LEFT_MIDDLE_DISTAL,
  LEFT_RING_PROXIMAL,
  LEFT_RING_INTERMEDIATE,
  LEFT_RING_DISTAL,
  LEFT_LITTLE_PROXIMAL,
  LEFT_LITTLE_INTERMEDIATE,
  LEFT_LITTLE_DISTAL,
  RIGHT_THUMB_METACARPAL,
  RIGHT_THUMB_PROXIMAL,
  RIGHT_THUMB_DISTAL,
  RIGHT_INDEX_PROXIMAL,
  RIGHT_INDEX_INTERMEDIATE,
  RIGHT_INDEX_DISTAL,
  RIGHT_MIDDLE_PROXIMAL,
  RIGHT_MIDDLE_INTERMEDIATE,
  RIGHT_MIDDLE_DISTAL,
  RIGHT_RING_PROXIMAL,
  RIGHT_RING_INTERMEDIATE,
  RIGHT_RING_DISTAL,
  RIGHT_LITTLE_PROXIMAL,
  RIGHT_LITTLE_INTERMEDIATE,
  RIGHT_LITTLE_DISTAL,
  UNKNOWN = std::numeric_limits<unsigned short>::max(),
};
inline std::optional<HumanBones>
ToVrmBone(srht::HumanoidBones src)
{
  switch (src) {
      // body: 6
    case HumanoidBones::HIPS:
      return HumanBones::hips;
    case HumanoidBones::SPINE:
      return HumanBones::spine;
    case HumanoidBones::CHEST:
      return HumanBones::chest;
    case HumanoidBones::UPPER_CHEST:
      return HumanBones::upperChest;
    case HumanoidBones::NECK:
      return HumanBones::neck;
    case HumanoidBones::HEAD:
      return HumanBones::head;
      // legs: 4 x 2
    case HumanoidBones::LEFT_UPPER_LEG:
      return HumanBones::leftUpperLeg;
    case HumanoidBones::LEFT_LOWER_LEG:
      return HumanBones::leftLowerLeg;
    case HumanoidBones::LEFT_FOOT:
      return HumanBones::leftFoot;
    case HumanoidBones::LEFT_TOES:
      return HumanBones::leftToes;
    case HumanoidBones::RIGHT_UPPER_LEG:
      return HumanBones::rightUpperLeg;
    case HumanoidBones::RIGHT_LOWER_LEG:
      return HumanBones::rightLowerLeg;
    case HumanoidBones::RIGHT_FOOT:
      return HumanBones::rightFoot;
    case HumanoidBones::RIGHT_TOES:
      return HumanBones::rightToes;
      // arms: 4 x 2
    case HumanoidBones::LEFT_SHOULDER:
      return HumanBones::leftShoulder;
    case HumanoidBones::LEFT_UPPER_ARM:
      return HumanBones::leftUpperArm;
    case HumanoidBones::LEFT_LOWER_ARM:
      return HumanBones::leftLowerArm;
    case HumanoidBones::LEFT_HAND:
      return HumanBones::leftHand;
    case HumanoidBones::RIGHT_SHOULDER:
      return HumanBones::rightShoulder;
    case HumanoidBones::RIGHT_UPPER_ARM:
      return HumanBones::rightUpperArm;
    case HumanoidBones::RIGHT_LOWER_ARM:
      return HumanBones::rightLowerArm;
    case HumanoidBones::RIGHT_HAND:
      return HumanBones::rightHand;
      // fingers: 3 x 5 x 2
    case HumanoidBones::LEFT_THUMB_METACARPAL:
      return HumanBones::leftThumbMetacarpal;
    case HumanoidBones::LEFT_THUMB_PROXIMAL:
      return HumanBones::leftThumbProximal;
    case HumanoidBones::LEFT_THUMB_DISTAL:
      return HumanBones::leftThumbDistal;
    case HumanoidBones::LEFT_INDEX_PROXIMAL:
      return HumanBones::leftIndexProximal;
    case HumanoidBones::LEFT_INDEX_INTERMEDIATE:
      return HumanBones::leftIndexIntermediate;
    case HumanoidBones::LEFT_INDEX_DISTAL:
      return HumanBones::leftIndexDistal;
    case HumanoidBones::LEFT_MIDDLE_PROXIMAL:
      return HumanBones::leftMiddleProximal;
    case HumanoidBones::LEFT_MIDDLE_INTERMEDIATE:
      return HumanBones::leftMiddleIntermediate;
    case HumanoidBones::LEFT_MIDDLE_DISTAL:
      return HumanBones::leftMiddleDistal;
    case HumanoidBones::LEFT_RING_PROXIMAL:
      return HumanBones::leftRingProximal;
    case HumanoidBones::LEFT_RING_INTERMEDIATE:
      return HumanBones::leftRingIntermediate;
    case HumanoidBones::LEFT_RING_DISTAL:
      return HumanBones::leftRingDistal;
    case HumanoidBones::LEFT_LITTLE_PROXIMAL:
      return HumanBones::leftLittleProximal;
    case HumanoidBones::LEFT_LITTLE_INTERMEDIATE:
      return HumanBones::leftLittleIntermediate;
    case HumanoidBones::LEFT_LITTLE_DISTAL:
      return HumanBones::leftLittleDistal;
    case HumanoidBones::RIGHT_THUMB_METACARPAL:
      return HumanBones::rightThumbMetacarpal;
    case HumanoidBones::RIGHT_THUMB_PROXIMAL:
      return HumanBones::rightThumbProximal;
    case HumanoidBones::RIGHT_THUMB_DISTAL:
      return HumanBones::rightThumbMetacarpal;
    case HumanoidBones::RIGHT_INDEX_PROXIMAL:
      return HumanBones::rightIndexProximal;
    case HumanoidBones::RIGHT_INDEX_INTERMEDIATE:
      return HumanBones::rightIndexIntermediate;
    case HumanoidBones::RIGHT_INDEX_DISTAL:
      return HumanBones::rightIndexDistal;
    case HumanoidBones::RIGHT_MIDDLE_PROXIMAL:
      return HumanBones::rightMiddleProximal;
    case HumanoidBones::RIGHT_MIDDLE_INTERMEDIATE:
      return HumanBones::rightMiddleIntermediate;
    case HumanoidBones::RIGHT_MIDDLE_DISTAL:
      return HumanBones::rightMiddleDistal;
    case HumanoidBones::RIGHT_RING_PROXIMAL:
      return HumanBones::rightRingProximal;
    case HumanoidBones::RIGHT_RING_INTERMEDIATE:
      return HumanBones::rightRingIntermediate;
    case HumanoidBones::RIGHT_RING_DISTAL:
      return HumanBones::rightRingDistal;
    case HumanoidBones::RIGHT_LITTLE_PROXIMAL:
      return HumanBones::rightLittleProximal;
    case HumanoidBones::RIGHT_LITTLE_INTERMEDIATE:
      return HumanBones::rightLittleIntermediate;
    case HumanoidBones::RIGHT_LITTLE_DISTAL:
      return HumanBones::rightLittleDistal;
  }

  return {};
}
inline HumanoidBones
FromVrmBone(HumanBones vrm_bone)
{
  switch (vrm_bone) {
    case HumanBones::hips:
      return HumanoidBones::HIPS;
    case HumanBones::spine:
      return HumanoidBones::SPINE;
    case HumanBones::chest:
      return HumanoidBones::CHEST;
    case HumanBones::upperChest:
      return HumanoidBones::UPPER_CHEST;
    case HumanBones::neck:
      return HumanoidBones::NECK;
    case HumanBones::head:
      return HumanoidBones::HEAD;
    case HumanBones::leftEye:
      return {};
    case HumanBones::rightEye:
      return {};
    case HumanBones::jaw:
      return {};
      // arms
    case HumanBones::leftShoulder:
      return HumanoidBones::LEFT_SHOULDER;
    case HumanBones::leftUpperArm:
      return HumanoidBones::LEFT_UPPER_ARM;
    case HumanBones::leftLowerArm:
      return HumanoidBones::LEFT_LOWER_ARM;
    case HumanBones::leftHand:
      return HumanoidBones::LEFT_HAND;
    case HumanBones::rightShoulder:
      return HumanoidBones::RIGHT_SHOULDER;
    case HumanBones::rightUpperArm:
      return HumanoidBones::RIGHT_UPPER_ARM;
    case HumanBones::rightLowerArm:
      return HumanoidBones::RIGHT_LOWER_ARM;
    case HumanBones::rightHand:
      return HumanoidBones::RIGHT_HAND;
      // legs
    case HumanBones::leftUpperLeg:
      return HumanoidBones::LEFT_UPPER_LEG;
    case HumanBones::leftLowerLeg:
      return HumanoidBones::LEFT_LOWER_LEG;
    case HumanBones::leftFoot:
      return HumanoidBones::LEFT_FOOT;
    case HumanBones::leftToes:
      return HumanoidBones::LEFT_TOES;
    case HumanBones::rightUpperLeg:
      return HumanoidBones::RIGHT_UPPER_LEG;
    case HumanBones::rightLowerLeg:
      return HumanoidBones::RIGHT_LOWER_LEG;
    case HumanBones::rightFoot:
      return HumanoidBones::RIGHT_FOOT;
    case HumanBones::rightToes:
      return HumanoidBones::RIGHT_TOES;
      // fingers
    case HumanBones::leftThumbMetacarpal:
      return HumanoidBones::LEFT_THUMB_METACARPAL;
    case HumanBones::leftThumbProximal:
      return HumanoidBones::LEFT_THUMB_PROXIMAL;
    case HumanBones::leftThumbDistal:
      return HumanoidBones::LEFT_THUMB_DISTAL;
    case HumanBones::leftIndexProximal:
      return HumanoidBones::LEFT_INDEX_PROXIMAL;
    case HumanBones::leftIndexIntermediate:
      return HumanoidBones::LEFT_INDEX_INTERMEDIATE;
    case HumanBones::leftIndexDistal:
      return HumanoidBones::LEFT_INDEX_DISTAL;
    case HumanBones::leftMiddleProximal:
      return HumanoidBones::LEFT_MIDDLE_PROXIMAL;
    case HumanBones::leftMiddleIntermediate:
      return HumanoidBones::LEFT_MIDDLE_INTERMEDIATE;
    case HumanBones::leftMiddleDistal:
      return HumanoidBones::LEFT_MIDDLE_DISTAL;
    case HumanBones::leftRingProximal:
      return HumanoidBones::LEFT_RING_PROXIMAL;
    case HumanBones::leftRingIntermediate:
      return HumanoidBones::LEFT_RING_INTERMEDIATE;
    case HumanBones::leftRingDistal:
      return HumanoidBones::LEFT_RING_DISTAL;
    case HumanBones::leftLittleProximal:
      return HumanoidBones::LEFT_LITTLE_PROXIMAL;
    case HumanBones::leftLittleIntermediate:
      return HumanoidBones::LEFT_LITTLE_INTERMEDIATE;
    case HumanBones::leftLittleDistal:
      return HumanoidBones::LEFT_LITTLE_DISTAL;
    case HumanBones::rightThumbMetacarpal:
      return HumanoidBones::RIGHT_THUMB_METACARPAL;
    case HumanBones::rightThumbProximal:
      return HumanoidBones::RIGHT_THUMB_PROXIMAL;
    case HumanBones::rightThumbDistal:
      return HumanoidBones::RIGHT_THUMB_DISTAL;
    case HumanBones::rightIndexProximal:
      return HumanoidBones::RIGHT_INDEX_PROXIMAL;
    case HumanBones::rightIndexIntermediate:
      return HumanoidBones::RIGHT_INDEX_INTERMEDIATE;
    case HumanBones::rightIndexDistal:
      return HumanoidBones::RIGHT_INDEX_DISTAL;
    case HumanBones::rightMiddleProximal:
      return HumanoidBones::RIGHT_MIDDLE_PROXIMAL;
    case HumanBones::rightMiddleIntermediate:
      return HumanoidBones::RIGHT_MIDDLE_INTERMEDIATE;
    case HumanBones::rightMiddleDistal:
      return HumanoidBones::RIGHT_MIDDLE_DISTAL;
    case HumanBones::rightRingProximal:
      return HumanoidBones::RIGHT_RING_PROXIMAL;
    case HumanBones::rightRingIntermediate:
      return HumanoidBones::RIGHT_RING_INTERMEDIATE;
    case HumanBones::rightRingDistal:
      return HumanoidBones::RIGHT_RING_DISTAL;
    case HumanBones::rightLittleProximal:
      return HumanoidBones::RIGHT_LITTLE_PROXIMAL;
    case HumanBones::rightLittleIntermediate:
      return HumanoidBones::RIGHT_LITTLE_INTERMEDIATE;
    case HumanBones::rightLittleDistal:
      return HumanoidBones::RIGHT_LITTLE_DISTAL;

    default:
      return {};
  }
}

union PackQuat
{
  uint32_t value;
};

struct JointDefinition
{
  // parentBone(-1 for root)
  uint16_t parentBoneIndex;
  // HumanBones or any number
  uint16_t boneType;
  float xFromParent;
  float yFromParent;
  float zFromParent;
};
static_assert(sizeof(JointDefinition) == 16, "JointDefintion");

enum class SkeletonFlags : uint32_t
{
  NONE = 0,
  // if hasInitialRotation PackQuat X jointCount for InitialRotation
  HAS_INITIAL_ROTATION = 0x1,
};
inline SkeletonFlags
operator|(SkeletonFlags lhs, SkeletonFlags rhs)
{
  return static_cast<SkeletonFlags>((uint16_t)lhs | (uint16_t)rhs);
}
inline SkeletonFlags
operator&(SkeletonFlags lhs, SkeletonFlags rhs)
{
  return static_cast<SkeletonFlags>((uint16_t)lhs & (uint16_t)rhs);
}

constexpr const char* SRHT_SKELETON_MAGIC1 = "SRHTSKL1";
struct SkeletonHeader
{
  uint16_t skeletonId = 0;
  uint16_t jointCount = 0;
  SkeletonFlags flags = {};
};
// continue JointDefinition X jointCount
static_assert(sizeof(SkeletonHeader) == 8, "Skeleton");

enum class FrameFlags : uint32_t
{
  NONE = 0,
  // enableed rotation is Quat32: disabled rotation is float4(x, y, z, w)
  USE_QUAT32 = 0x1,
};

constexpr const char* SRHT_FRAME_MAGIC1 = "SRHTFRM1";
struct FrameHeader
{
  // std::chrono::nanoseconds
  int64_t time;
  FrameFlags flags = {};
  uint16_t skeletonId = 0;
  // root position
  float x;
  float y;
  float z;
};
// continue PackQuat x SkeletonHeader::JointCount
static_assert(sizeof(FrameHeader) == 32, "FrameSize");

} // namespace srht
