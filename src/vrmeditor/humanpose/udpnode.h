#pragma once
#include "humanpose_stream.h"
#include "scenepreview.h"
#include "udp_receiver.h"
#include <vrm/scene.h>
#include <vrm/srht_update.h>

namespace humanpose {
struct UdpNode : public GraphNodeBase
{
  std::shared_ptr<libvrm::gltf::Scene> m_scene;
  std::shared_ptr<UdpReceiver> m_udp;
  ScenePreview m_preview;

  libvrm::Time m_lastTime = {};

  // constructor
  UdpNode(int id, std::string_view name)
    : GraphNodeBase(id, name)
  {
    m_scene = std::make_shared<libvrm::gltf::Scene>();

    // update preview
    m_scene->m_sceneUpdated.push_back(std::bind(
      &ScenePreview::OnSceneUpdated, &m_preview, std::placeholders::_1));

    m_udp = std::make_shared<UdpReceiver>();

    // update scene
    auto callback = [this](std::span<const uint8_t> data) {
      libvrm::srht::UpdateScene(m_scene, data);
      Outputs[0].Value = m_scene->UpdateHumanPose();
    };

    m_udp->Start(54345, callback);
  }

  void TimeUpdate(libvrm::Time time) override { m_udp->Update(); }

  void DrawContent() override { m_preview.Draw(); }
};
}
