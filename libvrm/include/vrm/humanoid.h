#pragma
#include "humanbones.h"
#include <optional>
#include <ostream>

namespace vrm {

struct Humanoid {
  std::optional<uint32_t> hips;
  std::optional<uint32_t> spine;
  std::optional<uint32_t> chest;
  std::optional<uint32_t> upperChest;
  std::optional<uint32_t> neck;
  std::optional<uint32_t> head;
  std::optional<uint32_t> leftEye;
  std::optional<uint32_t> rightEye;
  std::optional<uint32_t> jaw;
  // arms
  std::optional<uint32_t> leftShoulder;
  std::optional<uint32_t> leftUpperArm;
  std::optional<uint32_t> leftLowerArm;
  std::optional<uint32_t> leftHand;
  std::optional<uint32_t> rightShoulder;
  std::optional<uint32_t> rightUpperArm;
  std::optional<uint32_t> rightLowerArm;
  std::optional<uint32_t> rightHand;
  // legs
  std::optional<uint32_t> leftUpperLeg;
  std::optional<uint32_t> leftLowerLeg;
  std::optional<uint32_t> leftFoot;
  std::optional<uint32_t> leftToe;
  std::optional<uint32_t> rightUpperLeg;
  std::optional<uint32_t> rightLowerLeg;
  std::optional<uint32_t> rightFoot;
  std::optional<uint32_t> rightToe;
  // fingers
  std::optional<uint32_t> leftThumbMetacarpal;
  std::optional<uint32_t> leftThumbProximal;
  std::optional<uint32_t> leftThumbDistal;
  std::optional<uint32_t> leftIndexProximal;
  std::optional<uint32_t> leftIndexIntermediate;
  std::optional<uint32_t> leftIndexDistal;
  std::optional<uint32_t> leftMiddleProximal;
  std::optional<uint32_t> leftMiddleIntermediate;
  std::optional<uint32_t> leftMiddleDistal;
  std::optional<uint32_t> leftRingProximal;
  std::optional<uint32_t> leftRingIntermediate;
  std::optional<uint32_t> leftRingDistal;
  std::optional<uint32_t> leftLittleProximal;
  std::optional<uint32_t> leftLittleIntermediate;
  std::optional<uint32_t> leftLittleDistal;
  std::optional<uint32_t> rightThumbMetacarpal;
  std::optional<uint32_t> rightThumbProximal;
  std::optional<uint32_t> rightThumbDistal;
  std::optional<uint32_t> rightIndexProximal;
  std::optional<uint32_t> rightIndexIntermediate;
  std::optional<uint32_t> rightIndexDistal;
  std::optional<uint32_t> rightMiddleProximal;
  std::optional<uint32_t> rightMiddleIntermediate;
  std::optional<uint32_t> rightMiddleDistal;
  std::optional<uint32_t> rightRingProximal;
  std::optional<uint32_t> rightRingIntermediate;
  std::optional<uint32_t> rightRingDistal;
  std::optional<uint32_t> rightLittleProximal;
  std::optional<uint32_t> rightLittleIntermediate;
  std::optional<uint32_t> rightLittleDistal;

  bool setNode(std::string_view boneName, VrmVersion version,
               uint32_t nodeIndex) {
    if (auto bone = HumanBoneFromName(boneName, version)) {
      (*this)[(size_t)*bone] = nodeIndex;
      return true;
    } else {
      return false;
    }
  }

  std::optional<uint32_t> &operator[](size_t index) {
    return ((std::optional<uint32_t> *)this)[index];
  }
  const std::optional<uint32_t> &operator[](size_t index) const {
    return ((std::optional<uint32_t> *)this)[index];
  }
};
static_assert(sizeof(Humanoid) == sizeof(std::optional<uint32_t>) *
                                      (int)HumanBones::VRM_BONE_COUNT,
              "sizeof(Humanoid)");

inline std::ostream &operator<<(std::ostream &os, const Humanoid &rhs) {
  for (int i = 0; i < (int)HumanBones::VRM_BONE_COUNT; ++i) {
    if (rhs[i]) {
      os << HumanBoneToName((HumanBones)i) << ": " << *rhs[i] << std::endl;
    } else {
      os << HumanBoneToName((HumanBones)i) << ": null" << std::endl;
    }
  }
  return os;
}

} // namespace vrm
