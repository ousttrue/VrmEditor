#pragma once
#include "graphnode_base.h"

class UdpReceiver;
namespace libvrm {
namespace gltf {
struct Scene;
}
}

namespace glr {
struct ScenePreview;
}

namespace humanpose {
struct UdpNode : public GraphNodeBase
{
  std::shared_ptr<libvrm::gltf::Scene> m_scene;
  std::shared_ptr<UdpReceiver> m_udp;
  std::shared_ptr<glr::ScenePreview> m_preview;

  bool m_initialPose = false;

  // constructor
  UdpNode(int id, std::string_view name);
  void TimeUpdate(::libvrm::Time time) override;
  void DrawContent() override;
};
}
