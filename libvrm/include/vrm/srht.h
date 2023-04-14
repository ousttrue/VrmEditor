#pragma once
#include "quat_packer.h"

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
  UPPERCHEST,
  NECK,
  HEAD,
  // legs: 4 x 2
  LEFT_UPPERLEG,
  LEFT_LOWERLEG,
  LEFT_FOOT,
  LEFT_TOES,
  RIGHT_UPPERLEG,
  RIGHT_LOWERLEG,
  RIGHT_FOOT,
  RIGHT_TOES,
  // arms: 4 x 2
  LEFT_SHOULDER,
  LEFT_UPPERARM,
  LEFT_LOWERARM,
  LEFT_HAND,
  RIGHT_SHOULDER,
  RIGHT_UPPERARM,
  RIGHT_LOWERARM,
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
