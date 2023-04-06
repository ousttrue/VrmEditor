#include "vrm/bvhsolver.h"
#include "vrm/node.h"
#include <assert.h>
#include <iostream>

using GetJoint =
  std::function<const bvh::Joint*(const std::shared_ptr<gltf::Node>&)>;

const float DEFAULT_SIZE = 0.04f;

static void
CalcShape(const GetJoint& getJoint,
          const std::shared_ptr<gltf::Node>& node,
          float scaling)
{
  DirectX::XMStoreFloat4x4(
    &node->ShapeMatrix,
    DirectX::XMMatrixScaling(DEFAULT_SIZE, DEFAULT_SIZE, DEFAULT_SIZE));

  auto joint_ = getJoint(node);
  auto isRoot_ = joint_->index == 0;
  if (!isRoot_) {
    std::shared_ptr<gltf::Node> tail;
    switch (node->Children.size()) {
      case 0:
        return;

      case 1:
        tail = node->Children.front();
        break;

      default:
        for (auto& child : node->Children) {
          if (!tail) {
            tail = child;
          } else if (std::abs(getJoint(child)->localOffset.x) <
                     std::abs(getJoint(tail)->localOffset.x)) {
            // coose center node
            tail = child;
          }
        }
    }

    auto _Y = DirectX::XMFLOAT3(getJoint(tail)->localOffset.x * scaling,
                                getJoint(tail)->localOffset.y * scaling,
                                getJoint(tail)->localOffset.z * scaling);
    auto Y = DirectX::XMLoadFloat3(&_Y);

    auto length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Y));
    // std::cout << name_ << "=>" << tail->name_ << "=" << length <<
    // std::endl;
    Y = DirectX::XMVector3Normalize(Y);
    auto _Z = DirectX::XMFLOAT3(0, 0, 1);
    auto Z = DirectX::XMLoadFloat3(&_Z);
    auto X = DirectX::XMVector3Cross(Y, Z);
    Z = DirectX::XMVector3Cross(X, Y);

    auto center = DirectX::XMMatrixTranslation(0, 0.5f, 0);
    auto scale = DirectX::XMMatrixScaling(DEFAULT_SIZE, length, DEFAULT_SIZE);
    DirectX::XMFLOAT4 _(0, 0, 0, 1);
    auto r = DirectX::XMMATRIX(X, Y, Z, DirectX::XMLoadFloat4(&_));

    auto shape = center * scale * r;
    DirectX::XMStoreFloat4x4(&node->ShapeMatrix, shape);
  }

  for (auto& child : node->Children) {
    ::CalcShape(getJoint, child, scaling);
  }
}

// [x, y, z][c6][c5][c4][c3][c2][c1][parent][root]
static void
ResolveFrame(const GetJoint& getJoint,
             std::shared_ptr<gltf::Node>& node,
             const bvh::Frame& frame,
             DirectX::XMMATRIX m,
             float scaling,
             std::span<DirectX::XMFLOAT4X4>::iterator& out,
             std::span<DirectX::XMFLOAT4>::iterator& outLocal)
{
  auto joint = getJoint(node);
  auto transform = frame.Resolve(joint->channels);

  auto t = DirectX::XMMatrixTranslation(transform.Translation.x * scaling,
                                        transform.Translation.y * scaling,
                                        transform.Translation.z * scaling);
  // auto r =
  //     DirectX::XMLoadFloat3x3((const DirectX::XMFLOAT3X3
  //     *)&transform.Rotation);
  auto r = DirectX::XMMatrixRotationQuaternion(
    DirectX::XMLoadFloat4(&transform.Rotation));

  // DirectX::XMStoreFloat4(&*outLocal, DirectX::XMQuaternionRotationMatrix(r));
  *outLocal = transform.Rotation;

  auto local = r * t;

  m = local * m;
  auto shape = DirectX::XMLoadFloat4x4(&node->ShapeMatrix);
  DirectX::XMStoreFloat4x4(&*out, shape * m);
  ++out;
  ++outLocal;
  for (auto& child : node->Children) {
    ResolveFrame(getJoint, child, frame, m, scaling, out, outLocal);
  }
}

namespace bvh {

void
Solver::Initialize(const std::shared_ptr<Bvh>& bvh)
{
  m_bvh = bvh;
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
  auto node = std::make_shared<gltf::Node>(joint.name);
  nodes_.push_back(node);
  instances_.push_back({});
  localRotations.push_back({});

  if (nodes_.size() == 1) {
    root_ = node;
  } else {
    auto parent = nodes_[joint.parent];
    gltf::Node::AddChild(parent, node);
  }
}

void
Solver::CalcShape()
{
  GetJoint getJoint =
    [this](const std::shared_ptr<gltf::Node>& node) -> const Joint* {
    for (int i = 0; i < nodes_.size(); ++i) {
      if (nodes_[i] == node) {
        return &m_bvh->joints[i];
      }
    }
    assert(false);
    return nullptr;
  };

  ::CalcShape(getJoint, root_, scaling_);
}

std::span<DirectX::XMFLOAT4X4>
Solver::ResolveFrame(const Frame& frame)
{
  auto span = std::span(instances_);
  auto it = span.begin();
  auto t_span = std::span(localRotations);
  auto t = t_span.begin();

  GetJoint getJoint =
    [this](const std::shared_ptr<gltf::Node>& node) -> const Joint* {
    for (int i = 0; i < nodes_.size(); ++i) {
      if (nodes_[i] == node) {
        return &m_bvh->joints[i];
      }
    }
    return nullptr;
  };

  ::ResolveFrame(
    getJoint, root_, frame, DirectX::XMMatrixIdentity(), scaling_, it, t);
  assert(it == span.end());
  return instances_;
}
}