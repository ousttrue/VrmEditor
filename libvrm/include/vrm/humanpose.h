#pragma once
#include "humanbones.h"
#include <DirectXMath.h>
#include <span>

namespace libvrm {
namespace vrm {

struct HumanPose
{
  DirectX::XMFLOAT3 RootPosition;
  std::span<const vrm::HumanBones> Bones;
  std::span<const DirectX::XMFLOAT4> Rotations;
};

}
}
