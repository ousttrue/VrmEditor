#pragma once
#include "bvh.h"
#include <DirectXMath.h>
#include <list>
#include <memory>
#include <stack>
#include <vector>

namespace gltf {
struct Node;
}
namespace bvh {
class Solver
{
  std::shared_ptr<Bvh> m_bvh;
  std::shared_ptr<gltf::Node> root_;
  float scaling_ = 1.0f;

public:
  std::vector<std::shared_ptr<gltf::Node>> nodes_;
  std::vector<DirectX::XMFLOAT4X4> instances_;
  std::vector<DirectX::XMFLOAT4> localRotations;
  void Initialize(const std::shared_ptr<Bvh>& bvh);
  std::span<DirectX::XMFLOAT4X4> ResolveFrame(const Frame& frame);

private:
  void PushJoint(const Joint& joint);
  void CalcShape();
};
}
