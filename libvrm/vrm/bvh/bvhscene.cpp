#include "bvhscene.h"
#include "../node.h"

namespace libvrm::bvh {
// [x, y, z][c6][c5][c4][c3][c2][c1][parent][root]
void
UpdateSceneFromBvhFrame(const std::shared_ptr<RuntimeScene>& scene,
                        std::shared_ptr<RuntimeNode>& node,
                        const std::shared_ptr<bvh::Bvh>& bvh,
                        const bvh::Frame& frame,
                        float scaling)
{
  auto joint = &bvh->joints[*scene->IndexOf(node)];
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
UpdateSceneFromBvhFrame(const std::shared_ptr<RuntimeScene>& scene,
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
PushJoint(const std::shared_ptr<GltfRoot>& scene,
          const bvh::Joint& joint,
          float scaling)
{
  auto node = std::make_shared<Node>(joint.name);
  node->InitialTransform.Rotation = { 0, 0, 0, 1 };
  node->InitialTransform.Translation = joint.localOffset;
  node->InitialTransform.Translation.x *= scaling;
  node->InitialTransform.Translation.y *= scaling;
  node->InitialTransform.Translation.z *= scaling;

  scene->m_nodes.push_back(node);
  if (auto parent_index = joint.parent) {
    auto parent = scene->m_nodes[*parent_index];
    Node::AddChild(parent, node);
  } else {
    scene->m_roots.push_back(node);
  }
}

void
InitializeSceneFromBvh(const std::shared_ptr<GltfRoot>& scene,
                       const std::shared_ptr<bvh::Bvh>& bvh,
                       const std::shared_ptr<HumanBoneMap>& map)
{
  scene->m_title = "BVH";
  for (auto& joint : bvh->joints) {
    PushJoint(scene, joint, bvh->GuessScaling());
  };

  // assign human bone
  if (map) {
    for (auto& node : scene->m_nodes) {
      auto found = map->NameBoneMap.find(node->Name);
      if (found != map->NameBoneMap.end()) {
        node->Humanoid = found->second;
      }
    }
  }

  scene->m_roots[0]->CalcWorldInitialMatrix(true);
  // move ground
  auto bb = scene->GetBoundingBox();
  scene->m_roots[0]->InitialTransform.Translation.y -= bb.Min.y;
  scene->m_roots[0]->CalcWorldInitialMatrix(true);
  scene->m_roots[0]->CalcShape();
  // scene->RaiseSceneUpdated();
}

}
