#include "vrm/node.h"
#include "vrm/humanoid.h"
#include "vrm/scene.h"
#include "vrm/vrm0.h"
#include "vrm/vrm1.h"
#include <iostream>

namespace gltf {
Node::Node(std::string_view name)
  : Name(name)
{
}

const std::string&
Node::Label(const Scene& scene) const
{
  if (m_label.empty()) {
    std::stringstream ss;
    if (auto mesh_index = Mesh) {
      ss << "󰕣 ";
      auto mesh = scene.m_meshes[*mesh_index];
      if (mesh->m_morphTargets.size()) {
        ss << " ";
      }
    }
    // humanoid
    for (int i = 0; i < (int)vrm::HumanBones::VRM_BONE_COUNT; ++i) {
      if (auto node_index = scene.m_humanoid[i]) {
        auto bone_node = scene.m_nodes[*node_index];
        if (bone_node.get() == this) {
          // HumanBone = static_cast<vrm::HumanBones>(i);
          switch ((vrm::HumanBones)i) {
            case vrm::HumanBones::hips:
              ss << " ";
              break;
            case vrm::HumanBones::head:
              ss << "󱍞 ";
              break;
            case vrm::HumanBones::leftEye:
            case vrm::HumanBones::rightEye:
              ss << " ";
              break;
            case vrm::HumanBones::leftHand:
            case vrm::HumanBones::leftThumbMetacarpal:
            case vrm::HumanBones::leftThumbProximal:
            case vrm::HumanBones::leftThumbDistal:
            case vrm::HumanBones::leftIndexProximal:
            case vrm::HumanBones::leftIndexIntermediate:
            case vrm::HumanBones::leftIndexDistal:
            case vrm::HumanBones::leftMiddleProximal:
            case vrm::HumanBones::leftMiddleIntermediate:
            case vrm::HumanBones::leftMiddleDistal:
            case vrm::HumanBones::leftRingProximal:
            case vrm::HumanBones::leftRingIntermediate:
            case vrm::HumanBones::leftRingDistal:
            case vrm::HumanBones::leftLittleProximal:
            case vrm::HumanBones::leftLittleIntermediate:
            case vrm::HumanBones::leftLittleDistal:
              ss << "󰹆 ";
              break;
            case vrm::HumanBones::rightHand:
            case vrm::HumanBones::rightThumbMetacarpal:
            case vrm::HumanBones::rightThumbProximal:
            case vrm::HumanBones::rightThumbDistal:
            case vrm::HumanBones::rightIndexProximal:
            case vrm::HumanBones::rightIndexIntermediate:
            case vrm::HumanBones::rightIndexDistal:
            case vrm::HumanBones::rightMiddleProximal:
            case vrm::HumanBones::rightMiddleIntermediate:
            case vrm::HumanBones::rightMiddleDistal:
            case vrm::HumanBones::rightRingProximal:
            case vrm::HumanBones::rightRingIntermediate:
            case vrm::HumanBones::rightRingDistal:
            case vrm::HumanBones::rightLittleProximal:
            case vrm::HumanBones::rightLittleIntermediate:
            case vrm::HumanBones::rightLittleDistal:
              ss << "󰹇 ";
              break;
            default:
              ss << "󰂹 ";
              break;
          }
          break;
        }
      }
    }
    // vrm0
    if (auto vrm = scene.m_vrm0) {
      // spring
      if (vrm->m_springs.size()) {
        for (auto& spring : vrm->m_springs) {
          for (auto joint : spring->bones) {
            auto joint_node = scene.m_nodes[joint];
            if (joint_node.get() == this) {
              ss << "󰚟 ";
              break;
            }
          }
        }
      }
      // collider
      if (vrm->m_colliderGroups.size()) {
      }
    }

    ss << Name;
    m_label = ss.str();
  }
  return m_label;
}

void
Node::CalcInitialMatrix()
{
  WorldInitialTransform = WorldTransform;
  WorldInitialScale = WorldScale;
  InitialTransform = Transform;
  InitialScale = Scale;
}

void
Node::AddChild(const std::shared_ptr<Node>& parent,
               const std::shared_ptr<Node>& child)
{
  if (auto current_parent = child->Parent.lock()) {
    current_parent->Children.remove(child);
  }
  child->Parent = parent;
  parent->Children.push_back(child);
}

void
Node::CalcWorldMatrix(bool recursive)
{
  auto world = Matrix() * ParentWorldMatrix();

  DirectX::XMVECTOR s;
  DirectX::XMVECTOR r;
  DirectX::XMVECTOR t;
  assert(DirectX::XMMatrixDecompose(&s, &r, &t, world));
  DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&WorldScale, s);
  DirectX::XMStoreFloat4((DirectX::XMFLOAT4*)&WorldTransform.Rotation, r);
  DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&WorldTransform.Translation, t);

  if (recursive) {
    for (auto& child : Children) {
      child->CalcWorldMatrix(true);
    }
  }
}

bool
Node::SetLocalMatrix(const DirectX::XMMATRIX& local)
{
  DirectX::XMVECTOR s;
  DirectX::XMVECTOR r;
  DirectX::XMVECTOR t;
  if (!DirectX::XMMatrixDecompose(&s, &r, &t, local)) {
    return false;
  }
  DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&Scale, s);
  DirectX::XMStoreFloat4((DirectX::XMFLOAT4*)&Transform.Rotation, r);
  DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&Transform.Translation, t);
  return true;
}

bool
Node::SetWorldMatrix(const DirectX::XMMATRIX& world)
{
  auto parentMatrix = ParentWorldMatrix();
  auto inv = DirectX::XMMatrixInverse(nullptr, parentMatrix);
  auto local = world * inv;
  return SetLocalMatrix(local);
}

void
Node::SetWorldRotation(const DirectX::XMFLOAT4& world, bool recursive)
{
  auto parent = ParentWorldRotation();
  DirectX::XMStoreFloat4(
    &Transform.Rotation,
    DirectX::XMQuaternionMultiply(
      DirectX::XMLoadFloat4(&world),
      DirectX::XMQuaternionInverse(DirectX::XMLoadFloat4(&parent))));
  CalcWorldMatrix(recursive);
}

void
Node::SetWorldRotation(const DirectX::XMFLOAT4X4& world, bool recursive)
{
  auto parentMatrix = ParentWorldMatrix();
  auto inv = DirectX::XMMatrixInverse(nullptr, parentMatrix);
  auto local = DirectX::XMLoadFloat4x4(&world) * inv;

  DirectX::XMVECTOR s;
  DirectX::XMVECTOR r;
  DirectX::XMVECTOR t;
  DirectX::XMMatrixDecompose(&s, &r, &t, local);

  DirectX::XMStoreFloat4(&Transform.Rotation, r);

  CalcWorldMatrix(recursive);
}

void
Node::Print(int level)
{
  for (int i = 0; i < level; ++i) {
    std::cout << "  ";
  }
  std::cout << Name << std::endl;
  for (auto child : Children) {
    child->Print(level + 1);
  }
}
} // namespace gltf
