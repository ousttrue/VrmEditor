#include "udpnode.h"
#include "udp_receiver.h"
#include <vrm/scene.h>
#include <vrm/srht_update.h>

namespace humanpose {
// constructor
UdpNode::UdpNode(int id, std::string_view name)
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
  };

  m_udp->Start(54345, callback);
}

void
UdpNode::TimeUpdate(libvrm::Time time)
{
  m_udp->Update();

  if (m_initialPose) {
    m_scene->SetInitialPose();
  }
  Outputs[0].Value = m_scene->UpdateHumanPose();
}

void
UdpNode::DrawContent()
{
  ImGui::Checkbox("init pose", &m_initialPose);
  m_preview.Draw();
}

}
