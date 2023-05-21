#pragma once
#include "graphnode_base.h"
#include <vrm/bvh/bvh.h>
#include <vrm/bvh/humanbone_map.h>

namespace glr {
struct ScenePreview;
}

namespace runtimescene {
struct RuntimeScene;
}

namespace humanpose {
struct BvhNode : public GraphNodeBase
{
  std::shared_ptr<runtimescene::RuntimeScene> m_scene;
  std::shared_ptr<libvrm::bvh::Bvh> m_bvh;

  std::shared_ptr<glr::ScenePreview> m_preview;

  bool m_initialPose = false;

  // constructor
  BvhNode(int id, std::string_view name);
  void SetBvh(const std::shared_ptr<libvrm::bvh::Bvh>& bvh,
              const std::shared_ptr<libvrm::vrm::HumanBoneMap>& map);
  void TimeUpdate(libvrm::Time time) override;
  void DrawContent() override;
};
}
