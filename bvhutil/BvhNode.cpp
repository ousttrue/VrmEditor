#include "BvhNode.h"

const float DEFAULT_SIZE = 0.04f;

BvhNode::BvhNode(const BvhJoint &joint) : joint_(joint) {
  DirectX::XMStoreFloat4x4(
      &shape_,
      DirectX::XMMatrixScaling(DEFAULT_SIZE, DEFAULT_SIZE, DEFAULT_SIZE));
}

void BvhNode::CalcShape(float scaling) {
  auto isRoot_ = joint_.index == 0;
  if (!isRoot_) {
    std::shared_ptr<BvhNode> tail;
    switch (children_.size()) {
    case 0:
      return;

    case 1:
      tail = children_.front();
      break;

    default:
      for (auto &child : children_) {
        if (!tail) {
          tail = child;
        } else if (std::abs(child->joint_.localOffset.x) <
                   std::abs(tail->joint_.localOffset.x)) {
          // coose center node
          tail = child;
        }
      }
    }

    auto _Y = DirectX::XMFLOAT3(tail->joint_.localOffset.x * scaling,
                                tail->joint_.localOffset.y * scaling,
                                tail->joint_.localOffset.z * scaling);
    auto Y = DirectX::XMLoadFloat3(&_Y);
    DirectX::XMFLOAT3 length;
    DirectX::XMStoreFloat3(&length, DirectX::XMVector3Length(Y));
    // std::cout << name_ << "=>" << tail->name_ << "=" << length << std::endl;
    Y = DirectX::XMVector3Normalize(Y);
    DirectX::XMFLOAT3 _Z(0, 0, 1);
    auto Z = DirectX::XMLoadFloat3(&_Z);
    auto X = DirectX::XMVector3Cross(Y, Z);
    Z = DirectX::XMVector3Cross(X, Y);
    DirectX::XMMATRIX r;
    r.r[0] = X;
    r.r[1] = Y;
    r.r[2] = Z;
    r.r[3] = {0, 0, 0, 1};

    auto center = DirectX::XMMatrixTranslation(0, 0.5f, 0);
    auto scale = DirectX::XMMatrixScaling(DEFAULT_SIZE, length.x, DEFAULT_SIZE);

    auto shape = center * scale * r;
    DirectX::XMStoreFloat4x4(&shape_, shape);
  }

  for (auto &child : children_) {
    child->CalcShape(scaling);
  }
}

// [x, y, z][c6][c5][c4][c3][c2][c1][parent][root]
void BvhNode::ResolveFrame(const BvhFrame &frame, DirectX::XMMATRIX m,
                           float scaling,
                           std::span<DirectX::XMFLOAT4X4>::iterator &out) {
  auto [pos, rot] = frame.Resolve(joint_.channels);

  auto t = DirectX::XMMatrixTranslation(pos.x * scaling, pos.y * scaling,
                                        pos.z * scaling);
  auto r = DirectX::XMLoadFloat3x3((const DirectX::XMFLOAT3X3 *)&rot);
  auto local = r * t;

  m = local * m;
  auto shape = DirectX::XMLoadFloat4x4(&shape_);
  DirectX::XMStoreFloat4x4(&*out, shape * m);
  ++out;
  for (auto &child : children_) {
    child->ResolveFrame(frame, m, scaling, out);
  }
}
