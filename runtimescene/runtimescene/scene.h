#pragma once
#include "runtimescene/sprintjoint.h"
#include <unordered_map>
#include <vrm/mesh.h>
#include <vrm/scene.h>
#include <vrm/springbone.h>

namespace runtimescene {

using RenderFunc =
  std::function<void(const std::shared_ptr<libvrm::gltf::Mesh>&,
                     const libvrm::gltf::MeshInstance&,
                     const float[16])>;

struct RuntimeNode
{};

struct RuntimeMesh
{};

struct RuntimeScene
{
  libvrm::Time NextSpringDelta = libvrm::Time(0.0);
  std::shared_ptr<libvrm::gltf::Scene> m_lastScene;
  std::unordered_map<std::shared_ptr<libvrm::gltf::Node>,
                     std::shared_ptr<RuntimeSpringJoint>>
    m_jointMap;
  std::unordered_map<std::shared_ptr<libvrm::gltf::Mesh>,
                     std::shared_ptr<RuntimeMesh>>
    m_meshMap;
  std::unordered_map<std::shared_ptr<libvrm::gltf::Node>,
                     std::shared_ptr<RuntimeNode>>
    m_nodeMap;

  std::shared_ptr<RuntimeSpringJoint> GetRuntimeJoint(
    const libvrm::vrm::SpringJoint& joint);

  std::shared_ptr<RuntimeNode> GetRuntimeMesh(
    const std::shared_ptr<libvrm::gltf::Mesh>& mesh);

  std::shared_ptr<RuntimeNode> GetRuntimeNode(
    const std::shared_ptr<libvrm::gltf::Node>& node);

  void Render(const std::shared_ptr<libvrm::gltf::Scene>& scene,
              const RenderFunc& render,
              libvrm::IGizmoDrawer* gizmo);

  void SpringUpdate(const std::shared_ptr<libvrm::vrm::SpringSolver>& solver,
                    libvrm::Time deltaForSimulation);
  void SpringDrawGizmo(const std::shared_ptr<libvrm::vrm::SpringSolver>& solver,
                       libvrm::IGizmoDrawer* gizmo);
};

}
