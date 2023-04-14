#pragma once
#include <DirectXMath.h>
#include <span>
#include "humanbones.h"

namespace libvrm::vrm {

struct HumanPose
{
  DirectX::XMFLOAT3 RootPosition;
  std::span<const vrm::HumanBones> Bones;
  std::span<const DirectX::XMFLOAT4> Rotations;
};

}
