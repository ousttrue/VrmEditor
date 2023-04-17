#pragma once
#include "graphnode_base.h"
#include "scenepreview.h"

class UdpReceiver;
namespace libvrm {
namespace gltf {
struct Scene;
}
}
namespace humanpose {
struct UdpNode : public GraphNodeBase
{
  std::shared_ptr<libvrm::gltf::Scene> m_scene;
  std::shared_ptr<UdpReceiver> m_udp;
  ScenePreview m_preview;

  // constructor
  UdpNode(int id, std::string_view name);
  void TimeUpdate(::libvrm::Time time) override;
  void DrawContent() override { m_preview.Draw(); }
};
}
