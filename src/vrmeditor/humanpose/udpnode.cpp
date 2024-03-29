#include "udpnode.h"
#include "../view/scene_preview.h"
#include "udp_receiver.h"
#include <imgui.h>
#include <vrm/gltfroot.h>
#include <vrm/network/srht_update.h>
#include <vrm/runtime_scene.h>

namespace humanpose {
// constructor
UdpNode::UdpNode(int id, std::string_view name)
  : GraphNodeBase(id, name)
{
  auto table = std::make_shared<libvrm::GltfRoot>();
  m_scene = libvrm::RuntimeScene::Load(table);
  m_scene->m_base->m_title = "UDP";

  // update preview
  m_preview = std::make_shared<ScenePreview>();
  m_preview->Settings()->ShowCuber = true;
  m_preview->SetRuntime(m_scene);

  m_udp = std::make_shared<UdpReceiver>();

  // update scene
  auto callback = [this](std::span<const uint8_t> data) {
    libvrm::srht::UpdateScene(m_scene->m_base, data);
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
    m_scene->m_base->m_title.c_str(), color, sc.x, sc.y, 300, 300);
}
}
