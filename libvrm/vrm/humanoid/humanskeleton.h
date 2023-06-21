#pragma once
#include "humanbones.h"
#include <DirectXMath.h>
#include <array>
#include <vector>

namespace libvrm {

struct SkeletonBone
{
  HumanBones HumanBone;
  std::array<float, 3> WorldPosition;
  std::array<float, 4> WorldRotation;
  // root has -1
  uint32_t ParentIndex = -1;

  SkeletonBone() {}
  SkeletonBone(HumanBones bone,
               const DirectX::XMFLOAT3& pos,
               const DirectX::XMFLOAT4& rot)
    : HumanBone(bone)
    , WorldPosition({ pos.x, pos.y, pos.z })
    , WorldRotation({ rot.x, rot.y, rot.z, rot.w })
  {
  }

  DirectX::XMMATRIX Matrix() const;
  DirectX::XMMATRIX InverseMatrix() const;

  std::tuple<std::array<float, 3>, std::array<float, 4>> ToLocal(
    const SkeletonBone& parent);
};

// T-Pose representation
struct HumanSkeleton
{
  // 0 is root
  std::vector<SkeletonBone> Bones;
};

} // namespace
