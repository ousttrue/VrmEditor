#pragma once
#include "Bvh.h"
#include <list>
#include <memory>
#include <vector>

namespace bvh {
class Node
{
  const Joint& joint_;
  std::list<std::shared_ptr<Node>> children_;
  DirectX::XMFLOAT4X4 shape_;

public:
  Node(const Joint& joint);
  void AddChild(const std::shared_ptr<Node>& node)
  {
    children_.push_back(node);
  }
  void CalcShape(float scaling);
  void ResolveFrame(const Frame& frame,
                    DirectX::XMMATRIX m,
                    float scaling,
                    std::span<DirectX::XMFLOAT4X4>::iterator& out,
                    std::span<DirectX::XMFLOAT4>::iterator& outLocal);
};
}
