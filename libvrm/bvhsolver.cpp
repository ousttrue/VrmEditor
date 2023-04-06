#include "vrm/bvhsolver.h"
#include "vrm/bvhnode.h"
#include <assert.h>
#include <iostream>

namespace bvh {
void
Solver::Initialize(const std::shared_ptr<Bvh>& bvh)
{
  nodes_.clear();
  root_.reset();
  instances_.clear();

  scaling_ = bvh->GuessScaling();
  for (auto& joint : bvh->joints) {
    PushJoint(joint);
  };
  CalcShape();
}

void
Solver::PushJoint(const Joint& joint)
{
  auto node = std::make_shared<Node>(joint);
  nodes_.push_back(node);
  instances_.push_back({});
  localRotations.push_back({});

  if (nodes_.size() == 1) {
    root_ = node;
  } else {
    auto parent = nodes_[joint.parent];
    parent->AddChild(node);
  }
}

void
Solver::CalcShape()
{
  root_->CalcShape(scaling_);
}

std::span<DirectX::XMFLOAT4X4>
Solver::ResolveFrame(const Frame& frame)
{
  auto span = std::span(instances_);
  auto it = span.begin();
  auto t_span = std::span(localRotations);
  auto t = t_span.begin();
  root_->ResolveFrame(frame, DirectX::XMMatrixIdentity(), scaling_, it, t);
  assert(it == span.end());
  return instances_;
}
}
