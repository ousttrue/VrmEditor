#pragma once
#include <DirectXMath.h>
#include <memory>

namespace libvrm {
namespace gltf {
struct Node;
}

namespace vrm {
class SpringJoint
{
public:
  std::shared_ptr<gltf::Node> Head;
  std::shared_ptr<gltf::Node> Tail;
  // if Tail is nullptr
  DirectX::XMFLOAT3 LocalTailPosition;

  // 減衰[0~1]
  float DragForce = 0;
  // 剛性。初期姿勢への復元力[0~]
  // 増えれば増えるほど比率が高まる。
  float Stiffness = 0;
  // tail hit radius
  float Radius = 0;

public:
  SpringJoint(const std::shared_ptr<gltf::Node>& head,
              const std::shared_ptr<gltf::Node>& tail,
              const DirectX::XMFLOAT3& localTailPosition,
              float dragForce,
              float stiffness,
              float radius)
    : Head(head)
    , Tail(tail)
    , LocalTailPosition(localTailPosition)
    , DragForce(dragForce)
    , Stiffness(stiffness)
    , Radius(radius)
  {
  }
};

}
}