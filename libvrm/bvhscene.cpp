#include "vrm/bvhscene.h"
#include "vrm/node.h"

namespace libvrm::bvh {
// [x, y, z][c6][c5][c4][c3][c2][c1][parent][root]
void
UpdateSceneFromBvhFrame(const std::shared_ptr<gltf::Scene>& scene,
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
    UpdateSceneFromBvhFrame(scene, child, bvh, frame, scaling);
  }
}

void
UpdateSceneFromBvhFrame(const std::shared_ptr<gltf::Scene>& scene,
                        const std::shared_ptr<bvh::Bvh>& bvh,
                        Time time)
{
  if (scene->m_roots.empty()) {
    return;
  }
  auto index = bvh->TimeToIndex(time);
  auto frame = bvh->GetFrame(index);
  UpdateSceneFromBvhFrame(
    scene, scene->m_roots[0], bvh, frame, bvh->GuessScaling());
  scene->m_roots[0]->CalcWorldMatrix(true);
  scene->RaiseSceneUpdated();
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
  if (auto parent_index = joint.parent) {
    auto parent = scene->m_nodes[*parent_index];
    gltf::Node::AddChild(parent, node);
  } else {
    scene->m_roots.push_back(node);
  }
}

void
InitializeSceneFromBvh(const std::shared_ptr<gltf::Scene>& scene,
                       const std::shared_ptr<bvh::Bvh>& bvh,
                       const std::shared_ptr<vrm::HumanBoneMap>& map)
{
  scene->m_title = "BVH";
  for (auto& joint : bvh->joints) {
    PushJoint(scene, joint, bvh->GuessScaling());
  };

  // assign human bone
  for (auto& node : scene->m_nodes) {
    auto found = map->NameBoneMap.find(node->Name);
    if (found != map->NameBoneMap.end()) {
      node->Humanoid = libvrm::gltf::NodeHumanoidInfo{
        .HumanBone = found->second,
      };
    }
  }

  scene->m_roots[0]->CalcWorldMatrix(true);
  // move ground
  auto bb = scene->GetBoundingBox();
  scene->m_roots[0]->Transform.Translation.y -= bb.Min.y;

  scene->InitializeNodes();
  scene->RaiseSceneUpdated();
}

}
