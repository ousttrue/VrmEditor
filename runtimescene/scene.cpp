#include "runtimescene/scene.h"
#include <vrm/skin.h>

namespace runtimescene {

std::shared_ptr<RuntimeSpringJoint>
RuntimeScene::GetRuntimeJoint(const libvrm::vrm::SpringJoint& joint)
{
  auto found = m_jointMap.find(joint.Head);
  if (found != m_jointMap.end()) {
    return found->second;
  }

  auto runtime = std::make_shared<RuntimeSpringJoint>(joint);
  m_jointMap.insert({ joint.Head, runtime });
  return runtime;
}

void
RuntimeScene::Render(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                     const RenderFunc& render,
                     libvrm::IGizmoDrawer* gizmo)
{
  if (scene != m_lastScene) {
    // clear
    m_jointMap.clear();
    m_lastScene = scene;
  }

  // update order
  // 1. ヒューマノイドボーンを解決
  // 2. 頭の位置が決まるのでLookAtを解決
  //  * Bone型 => leftEye, rightEye ボーンを回転
  //  * Expression型 => 次項
  // 3. ExpressionUpdate
  //  * Expression を Apply する
  // 4. コンストレイントを解決
  // 5. SpringBoneを解決

  // constraint
  for (auto& node : scene->m_nodes) {
    if (auto constraint = node->Constraint) {
      constraint->Process(node);
    }
  }
  for (auto& root : scene->m_roots) {
    root->CalcWorldMatrix(true);
  }

  // springbone
  for (auto& spring : scene->m_springSolvers) {
    SpringUpdate(spring, NextSpringDelta);
  }
  NextSpringDelta = {};

  if (scene->m_expressions) {
    // VRM0 expression to morphTarget
    auto nodeToIndex = [nodes = scene->m_nodes,
                        expressions = scene->m_expressions](
                         const std::shared_ptr<libvrm::gltf::Node>& node) {
      for (uint32_t i = 0; i < nodes.size(); ++i) {
        if (node == nodes[i]) {
          return i;
        }
      }
      return (uint32_t)-1;
    };
    for (auto& [k, v] : scene->m_expressions->EvalMorphTargetMap(nodeToIndex)) {
      auto& morph_node = scene->m_nodes[k.NodeIndex];
      morph_node->MeshInstance->weights[k.MorphIndex] = v;
    }
  }

  // skinning
  for (auto& node : scene->m_nodes) {
    if (auto mesh_index = node->Mesh) {
      auto mesh = scene->m_meshes[*mesh_index];

      // mesh animation
      std::span<DirectX::XMFLOAT4X4> skinningMatrices;

      // skinning
      if (auto skin = node->Skin) {
        skin->CurrentMatrices.resize(skin->BindMatrices.size());

        auto rootInverse = DirectX::XMMatrixIdentity();
        if (auto root_index = skin->Root) {
          rootInverse = DirectX::XMMatrixInverse(nullptr, node->WorldMatrix());
        }

        for (int i = 0; i < skin->Joints.size(); ++i) {
          auto node = scene->m_nodes[skin->Joints[i]];
          auto m = skin->BindMatrices[i];
          DirectX::XMStoreFloat4x4(&skin->CurrentMatrices[i],
                                   DirectX::XMLoadFloat4x4(&m) *
                                     node->WorldMatrix() * rootInverse);
        }

        skinningMatrices = skin->CurrentMatrices;
      }

      // apply morphtarget & skinning
      node->MeshInstance->applyMorphTargetAndSkinning(*mesh, skinningMatrices);
    }
  }
  DirectX::XMFLOAT4X4 m;
  for (auto& node : scene->m_nodes) {
    if (auto mesh_index = node->Mesh) {
      auto mesh = scene->m_meshes[*mesh_index];
      DirectX::XMStoreFloat4x4(&m, node->WorldMatrix());
      render(mesh, *node->MeshInstance, &m._11);
    }
  }

  for (auto& spring : scene->m_springSolvers) {
    SpringDrawGizmo(spring, gizmo);
  }
  for (auto& collider : scene->m_springColliders) {
    collider->DrawGizmo(gizmo);
  }
}

void
RuntimeScene::SpringUpdate(
  const std::shared_ptr<libvrm::vrm::SpringSolver>& solver,
  libvrm::Time delta)
{
  bool doUpdate = delta.count() > 0;
  if (!doUpdate) {
    return;
  }

  for (auto& joint : solver->Joints) {
    solver->Collision->Clear();
    auto runtime = GetRuntimeJoint(joint);
    runtime->Update(joint, delta, solver->Collision.get());
  }
}

void
RuntimeScene::SpringDrawGizmo(
  const std::shared_ptr<libvrm::vrm::SpringSolver>& solver,
  libvrm::IGizmoDrawer* gizmo)
{
  for (auto& joint : solver->Joints) {
    auto runtime = GetRuntimeJoint(joint);
    runtime->DrawGizmo(joint, gizmo);
  }
}

}
