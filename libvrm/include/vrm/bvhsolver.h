#pragma once
#include "bvh.h"
#include <DirectXMath.h>
#include <list>
#include <memory>
#include <stack>
#include <vector>

namespace gltf {
struct Scene;
struct Node;
}
namespace bvh {
class Solver
{
  std::shared_ptr<Bvh> m_bvh;
  float m_scaling = 1.0f;

public:
  std::shared_ptr<gltf::Scene> Scene;
  std::vector<DirectX::XMFLOAT4X4> Instances;
  std::vector<DirectX::XMFLOAT4> LocalRotations;
  void Initialize(const std::shared_ptr<Bvh>& bvh);
  std::span<DirectX::XMFLOAT4X4> ResolveFrame(const Frame& frame);

private:
  void PushJoint(const Joint& joint);
  void CalcShape();
  const Joint* GetJoint(const std::shared_ptr<gltf::Node>& node) const;
};
}
