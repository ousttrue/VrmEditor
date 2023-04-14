#pragma once
#include "humanbones.h"
#include "quat_packer.h"
#include <stdint.h>

//
// SingleRootHierarchicalTransformation
//
namespace libvrm::srht {

enum class HumanoidBones : uint16_t
{
  UNKNOWN = 0,
  // body: 6
  HIPS = 1,
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
};
inline std::optional<vrm::HumanBones>
ToVrmBone(srht::HumanoidBones src)
{
  switch (src) {
      // body: 6
    case HumanoidBones::HIPS:
      return vrm::HumanBones::hips;
    case HumanoidBones::SPINE:
      return vrm::HumanBones::spine;
    case HumanoidBones::CHEST:
      return vrm::HumanBones::chest;
    case HumanoidBones::UPPER_CHEST:
      return vrm::HumanBones::upperChest;
    case HumanoidBones::NECK:
      return vrm::HumanBones::neck;
    case HumanoidBones::HEAD:
      return vrm::HumanBones::head;
      // legs: 4 x 2
    case HumanoidBones::LEFT_UPPER_LEG:
      return vrm::HumanBones::leftUpperLeg;
    case HumanoidBones::LEFT_LOWER_LEG:
      return vrm::HumanBones::leftLowerLeg;
    case HumanoidBones::LEFT_FOOT:
      return vrm::HumanBones::leftFoot;
    case HumanoidBones::LEFT_TOES:
      return vrm::HumanBones::leftToes;
    case HumanoidBones::RIGHT_UPPER_LEG:
      return vrm::HumanBones::rightUpperLeg;
    case HumanoidBones::RIGHT_LOWER_LEG:
      return vrm::HumanBones::rightLowerLeg;
    case HumanoidBones::RIGHT_FOOT:
      return vrm::HumanBones::rightFoot;
    case HumanoidBones::RIGHT_TOES:
      return vrm::HumanBones::rightToes;
      // arms: 4 x 2
    case HumanoidBones::LEFT_SHOULDER:
      return vrm::HumanBones::leftShoulder;
    case HumanoidBones::LEFT_UPPER_ARM:
      return vrm::HumanBones::leftUpperArm;
    case HumanoidBones::LEFT_LOWER_ARM:
      return vrm::HumanBones::leftLowerArm;
    case HumanoidBones::LEFT_HAND:
      return vrm::HumanBones::leftHand;
    case HumanoidBones::RIGHT_SHOULDER:
      return vrm::HumanBones::rightShoulder;
    case HumanoidBones::RIGHT_UPPER_ARM:
      return vrm::HumanBones::rightUpperArm;
    case HumanoidBones::RIGHT_LOWER_ARM:
      return vrm::HumanBones::rightLowerArm;
    case HumanoidBones::RIGHT_HAND:
      return vrm::HumanBones::rightHand;
      // fingers: 3 x 5 x 2
    case HumanoidBones::LEFT_THUMB_METACARPAL:
      return vrm::HumanBones::leftThumbMetacarpal;
    case HumanoidBones::LEFT_THUMB_PROXIMAL:
      return vrm::HumanBones::leftThumbProximal;
    case HumanoidBones::LEFT_THUMB_DISTAL:
      return vrm::HumanBones::leftThumbDistal;
    case HumanoidBones::LEFT_INDEX_PROXIMAL:
      return vrm::HumanBones::leftIndexProximal;
    case HumanoidBones::LEFT_INDEX_INTERMEDIATE:
      return vrm::HumanBones::leftIndexIntermediate;
    case HumanoidBones::LEFT_INDEX_DISTAL:
      return vrm::HumanBones::leftIndexDistal;
    case HumanoidBones::LEFT_MIDDLE_PROXIMAL:
      return vrm::HumanBones::leftMiddleProximal;
    case HumanoidBones::LEFT_MIDDLE_INTERMEDIATE:
      return vrm::HumanBones::leftMiddleIntermediate;
    case HumanoidBones::LEFT_MIDDLE_DISTAL:
      return vrm::HumanBones::leftMiddleDistal;
    case HumanoidBones::LEFT_RING_PROXIMAL:
      return vrm::HumanBones::leftRingProximal;
    case HumanoidBones::LEFT_RING_INTERMEDIATE:
      return vrm::HumanBones::leftRingIntermediate;
    case HumanoidBones::LEFT_RING_DISTAL:
      return vrm::HumanBones::leftRingDistal;
    case HumanoidBones::LEFT_LITTLE_PROXIMAL:
      return vrm::HumanBones::leftLittleProximal;
    case HumanoidBones::LEFT_LITTLE_INTERMEDIATE:
      return vrm::HumanBones::leftLittleIntermediate;
    case HumanoidBones::LEFT_LITTLE_DISTAL:
      return vrm::HumanBones::leftLittleDistal;
    case HumanoidBones::RIGHT_THUMB_METACARPAL:
      return vrm::HumanBones::rightThumbMetacarpal;
    case HumanoidBones::RIGHT_THUMB_PROXIMAL:
      return vrm::HumanBones::rightThumbProximal;
    case HumanoidBones::RIGHT_THUMB_DISTAL:
      return vrm::HumanBones::rightThumbMetacarpal;
    case HumanoidBones::RIGHT_INDEX_PROXIMAL:
      return vrm::HumanBones::rightIndexProximal;
    case HumanoidBones::RIGHT_INDEX_INTERMEDIATE:
      return vrm::HumanBones::rightIndexIntermediate;
    case HumanoidBones::RIGHT_INDEX_DISTAL:
      return vrm::HumanBones::rightIndexDistal;
    case HumanoidBones::RIGHT_MIDDLE_PROXIMAL:
      return vrm::HumanBones::rightMiddleProximal;
    case HumanoidBones::RIGHT_MIDDLE_INTERMEDIATE:
      return vrm::HumanBones::rightMiddleIntermediate;
    case HumanoidBones::RIGHT_MIDDLE_DISTAL:
      return vrm::HumanBones::rightMiddleDistal;
    case HumanoidBones::RIGHT_RING_PROXIMAL:
      return vrm::HumanBones::rightRingProximal;
    case HumanoidBones::RIGHT_RING_INTERMEDIATE:
      return vrm::HumanBones::rightRingIntermediate;
    case HumanoidBones::RIGHT_RING_DISTAL:
      return vrm::HumanBones::rightRingDistal;
    case HumanoidBones::RIGHT_LITTLE_PROXIMAL:
      return vrm::HumanBones::rightLittleProximal;
    case HumanoidBones::RIGHT_LITTLE_INTERMEDIATE:
      return vrm::HumanBones::rightLittleIntermediate;
    case HumanoidBones::RIGHT_LITTLE_DISTAL:
      return vrm::HumanBones::rightLittleDistal;
  }

  return {};
}
inline HumanoidBones
FromVrmBone(vrm::HumanBones vrm_bone)
{
  switch (vrm_bone) {
    case vrm::HumanBones::hips:
      return HumanoidBones::HIPS;
    case vrm::HumanBones::spine:
      return HumanoidBones::SPINE;
    case vrm::HumanBones::chest:
      return HumanoidBones::CHEST;
    case vrm::HumanBones::upperChest:
      return HumanoidBones::UPPER_CHEST;
    case vrm::HumanBones::neck:
      return HumanoidBones::NECK;
    case vrm::HumanBones::head:
      return HumanoidBones::HEAD;
    case vrm::HumanBones::leftEye:
      return {};
    case vrm::HumanBones::rightEye:
      return {};
    case vrm::HumanBones::jaw:
      return {};
      // arms
    case vrm::HumanBones::leftShoulder:
      return HumanoidBones::LEFT_SHOULDER;
    case vrm::HumanBones::leftUpperArm:
      return HumanoidBones::LEFT_UPPER_ARM;
    case vrm::HumanBones::leftLowerArm:
      return HumanoidBones::LEFT_LOWER_ARM;
    case vrm::HumanBones::leftHand:
      return HumanoidBones::LEFT_HAND;
    case vrm::HumanBones::rightShoulder:
      return HumanoidBones::RIGHT_SHOULDER;
    case vrm::HumanBones::rightUpperArm:
      return HumanoidBones::RIGHT_UPPER_ARM;
    case vrm::HumanBones::rightLowerArm:
      return HumanoidBones::RIGHT_LOWER_ARM;
    case vrm::HumanBones::rightHand:
      return HumanoidBones::RIGHT_HAND;
      // legs
    case vrm::HumanBones::leftUpperLeg:
      return HumanoidBones::LEFT_UPPER_LEG;
    case vrm::HumanBones::leftLowerLeg:
      return HumanoidBones::LEFT_LOWER_LEG;
    case vrm::HumanBones::leftFoot:
      return HumanoidBones::LEFT_FOOT;
    case vrm::HumanBones::leftToes:
      return HumanoidBones::LEFT_TOES;
    case vrm::HumanBones::rightUpperLeg:
      return HumanoidBones::RIGHT_UPPER_LEG;
    case vrm::HumanBones::rightLowerLeg:
      return HumanoidBones::RIGHT_LOWER_LEG;
    case vrm::HumanBones::rightFoot:
      return HumanoidBones::RIGHT_FOOT;
    case vrm::HumanBones::rightToes:
      return HumanoidBones::RIGHT_TOES;
      // fingers
    case vrm::HumanBones::leftThumbMetacarpal:
      return HumanoidBones::LEFT_THUMB_METACARPAL;
    case vrm::HumanBones::leftThumbProximal:
      return HumanoidBones::LEFT_THUMB_PROXIMAL;
    case vrm::HumanBones::leftThumbDistal:
      return HumanoidBones::LEFT_THUMB_DISTAL;
    case vrm::HumanBones::leftIndexProximal:
      return HumanoidBones::LEFT_INDEX_PROXIMAL;
    case vrm::HumanBones::leftIndexIntermediate:
      return HumanoidBones::LEFT_INDEX_INTERMEDIATE;
    case vrm::HumanBones::leftIndexDistal:
      return HumanoidBones::LEFT_INDEX_DISTAL;
    case vrm::HumanBones::leftMiddleProximal:
      return HumanoidBones::LEFT_MIDDLE_PROXIMAL;
    case vrm::HumanBones::leftMiddleIntermediate:
      return HumanoidBones::LEFT_MIDDLE_INTERMEDIATE;
    case vrm::HumanBones::leftMiddleDistal:
      return HumanoidBones::LEFT_MIDDLE_DISTAL;
    case vrm::HumanBones::leftRingProximal:
      return HumanoidBones::LEFT_RING_PROXIMAL;
    case vrm::HumanBones::leftRingIntermediate:
      return HumanoidBones::LEFT_RING_INTERMEDIATE;
    case vrm::HumanBones::leftRingDistal:
      return HumanoidBones::LEFT_RING_DISTAL;
    case vrm::HumanBones::leftLittleProximal:
      return HumanoidBones::LEFT_LITTLE_PROXIMAL;
    case vrm::HumanBones::leftLittleIntermediate:
      return HumanoidBones::LEFT_LITTLE_INTERMEDIATE;
    case vrm::HumanBones::leftLittleDistal:
      return HumanoidBones::LEFT_LITTLE_DISTAL;
    case vrm::HumanBones::rightThumbMetacarpal:
      return HumanoidBones::RIGHT_THUMB_METACARPAL;
    case vrm::HumanBones::rightThumbProximal:
      return HumanoidBones::RIGHT_THUMB_PROXIMAL;
    case vrm::HumanBones::rightThumbDistal:
      return HumanoidBones::RIGHT_THUMB_DISTAL;
    case vrm::HumanBones::rightIndexProximal:
      return HumanoidBones::RIGHT_INDEX_PROXIMAL;
    case vrm::HumanBones::rightIndexIntermediate:
      return HumanoidBones::RIGHT_INDEX_INTERMEDIATE;
    case vrm::HumanBones::rightIndexDistal:
      return HumanoidBones::RIGHT_INDEX_DISTAL;
    case vrm::HumanBones::rightMiddleProximal:
      return HumanoidBones::RIGHT_MIDDLE_PROXIMAL;
    case vrm::HumanBones::rightMiddleIntermediate:
      return HumanoidBones::RIGHT_MIDDLE_INTERMEDIATE;
    case vrm::HumanBones::rightMiddleDistal:
      return HumanoidBones::RIGHT_MIDDLE_DISTAL;
    case vrm::HumanBones::rightRingProximal:
      return HumanoidBones::RIGHT_RING_PROXIMAL;
    case vrm::HumanBones::rightRingIntermediate:
      return HumanoidBones::RIGHT_RING_INTERMEDIATE;
    case vrm::HumanBones::rightRingDistal:
      return HumanoidBones::RIGHT_RING_DISTAL;
    case vrm::HumanBones::rightLittleProximal:
      return HumanoidBones::RIGHT_LITTLE_PROXIMAL;
    case vrm::HumanBones::rightLittleIntermediate:
      return HumanoidBones::RIGHT_LITTLE_INTERMEDIATE;
    case vrm::HumanBones::rightLittleDistal:
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
