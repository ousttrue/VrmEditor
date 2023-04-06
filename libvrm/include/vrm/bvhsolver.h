#pragma once
#include "Bvh.h"
#include <DirectXMath.h>
#include <list>
#include <memory>
#include <stack>
#include <vector>

class BvhNode;
class BvhSolver {
  std::vector<std::shared_ptr<BvhNode>> nodes_;
  std::shared_ptr<BvhNode> root_;
  float scaling_ = 1.0f;

public:
  std::vector<DirectX::XMFLOAT4X4> instances_;
  std::vector<DirectX::XMFLOAT4> localRotations;
  void Initialize(const std::shared_ptr<Bvh> &bvh);
  std::span<DirectX::XMFLOAT4X4> ResolveFrame(const BvhFrame &frame);

private:
  void PushJoint(const BvhJoint &joint);
  void CalcShape();
};
