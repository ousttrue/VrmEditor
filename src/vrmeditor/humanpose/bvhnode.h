#pragma once
#include "graphnode_base.h"
#include <vrm/bvh.h>
#include <vrm/humanbone_map.h>
#include <vrm/scene.h>

namespace glr {
struct CuberPreview;
}

namespace humanpose {
struct BvhNode : public GraphNodeBase
{
  std::shared_ptr<libvrm::gltf::Scene> m_scene;
  std::shared_ptr<libvrm::bvh::Bvh> m_bvh;

  std::shared_ptr<glr::CuberPreview> m_preview;

  bool m_initialPose = false;

  // constructor
  BvhNode(int id, std::string_view name);
  void SetBvh(const std::shared_ptr<libvrm::bvh::Bvh>& bvh,
              const std::shared_ptr<libvrm::vrm::HumanBoneMap>& map);
  void TimeUpdate(libvrm::Time time) override;
  void DrawContent() override;
};
}
