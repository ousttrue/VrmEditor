#pragma once
#include "Bvh.h"
#include <DirectXMath.h>
#include <list>
#include <memory>
#include <stack>
#include <vector>

namespace bvh {
class Node;
class Solver
{
  std::vector<std::shared_ptr<Node>> nodes_;
  std::shared_ptr<Node> root_;
  float scaling_ = 1.0f;

public:
  std::vector<DirectX::XMFLOAT4X4> instances_;
  std::vector<DirectX::XMFLOAT4> localRotations;
  void Initialize(const std::shared_ptr<Bvh>& bvh);
  std::span<DirectX::XMFLOAT4X4> ResolveFrame(const Frame& frame);

private:
  void PushJoint(const Joint& joint);
  void CalcShape();
};
}
