#include "udpnode.h"
#include "udp_receiver.h"
#include <glr/scene_preview.h>
#include <grapho/orbitview.h>
#include <imgui.h>
#include <vrm/runtimescene/scene.h>
#include <vrm/scene.h>
#include <vrm/srht_update.h>

namespace humanpose {
// constructor
UdpNode::UdpNode(int id, std::string_view name)
  : GraphNodeBase(id, name)
{
  auto table = std::make_shared<libvrm::gltf::Scene>();
  m_scene = std::make_shared<runtimescene::RuntimeScene>(table);
  m_scene->m_table->m_title = "UDP";

  // update preview
  m_preview = std::make_shared<glr::ScenePreview>(m_scene);

  m_udp = std::make_shared<UdpReceiver>();

  // update scene
  auto callback = [this](std::span<const uint8_t> data) {
    libvrm::srht::UpdateScene(m_scene->m_table, data);
  };

  m_udp->Start(54345, callback);
}

void
UdpNode::TimeUpdate(libvrm::Time time)
{
  m_udp->Update();

  if (m_initialPose) {
    // m_scene->m_table->SetInitialPose();
  }
  // Outputs[0].Value = m_scene->m_table->UpdateHumanPose();
}

void
UdpNode::DrawContent()
{
  ImGui::Checkbox("init pose", &m_initialPose);
  static float color[] = {
    0.2f,
    0.2f,
    0.2f,
    1.0f,
  };
  auto sc = ImGui::GetCursorScreenPos();
  m_preview->ShowScreenRect(
    m_scene->m_table->m_title.c_str(), color, sc.x, sc.y, 300, 300);
}

}
