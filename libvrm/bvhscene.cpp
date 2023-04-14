#include "vrm/bvhscene.h"
#include "vrm/node.h"

namespace libvrm::bvh {
// [x, y, z][c6][c5][c4][c3][c2][c1][parent][root]
void
ResolveFrame(const std::shared_ptr<gltf::Scene>& scene,
             std::shared_ptr<gltf::Node>& node,
             const std::shared_ptr<bvh::Bvh>& bvh,
             const bvh::Frame& frame,
             float scaling)
{
  auto joint = &bvh->joints[scene->GetNodeIndex(node)];
  auto transform = frame.Resolve(joint->channels);
  node->Transform.Translation = transform.Translation;
  node->Transform.Translation.x *= scaling;
  node->Transform.Translation.y *= scaling;
  node->Transform.Translation.z *= scaling;
  node->Transform.Rotation = transform.Rotation;
  for (auto& child : node->Children) {
    ResolveFrame(scene, child, bvh, frame, scaling);
  }
}

void
ResolveFrame(const std::shared_ptr<gltf::Scene>& scene,
             const std::shared_ptr<bvh::Bvh>& bvh,
             Time time)
{
  if (scene->m_roots.empty()) {
    return;
  }
  auto index = bvh->TimeToIndex(time);
  auto frame = bvh->GetFrame(index);
  ResolveFrame(scene, scene->m_roots[0], bvh, frame, bvh->GuessScaling());
}

static void
PushJoint(const std::shared_ptr<gltf::Scene>& scene,
          const bvh::Joint& joint,
          float scaling)
{
  auto node = std::make_shared<gltf::Node>(joint.name);
  node->Transform.Rotation = { 0, 0, 0, 1 };
  node->Transform.Translation = joint.localOffset;
  node->Transform.Translation.x *= scaling;
  node->Transform.Translation.y *= scaling;
  node->Transform.Translation.z *= scaling;

  scene->m_nodes.push_back(node);
  if (scene->m_nodes.size() == 1) {
    scene->m_roots.push_back(node);
  } else {
    auto parent = scene->m_nodes[joint.parent];
    gltf::Node::AddChild(parent, node);
  }
}

void
SetBvh(const std::shared_ptr<gltf::Scene>& scene,
       const std::shared_ptr<bvh::Bvh>& bvh)
{
  for (auto& joint : bvh->joints) {
    PushJoint(scene, joint, bvh->GuessScaling());
  };
  scene->InitNodes();
}

}
