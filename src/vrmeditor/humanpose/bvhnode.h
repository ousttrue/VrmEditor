#pragma once
#include "scenepreview.h"
#include <vrm/bvh.h>
#include "graphnode_base.h"
#include <vrm/humanbone_map.h>

namespace humanpose {
struct BvhNode : public GraphNodeBase
{
  std::shared_ptr<libvrm::gltf::Scene> m_scene;
  std::shared_ptr<libvrm::bvh::Bvh> m_bvh;

  ScenePreview m_preview;

  bool m_initialPose = false;

  // constructor
  BvhNode(int id, std::string_view name);
  void SetBvh(const std::shared_ptr<libvrm::bvh::Bvh>& bvh,
              const std::shared_ptr<libvrm::vrm::HumanBoneMap>& map);
  void TimeUpdate(libvrm::Time time) override;
  void DrawContent() override;
};
}
