#pragma once
#include "scenepreview.h"
#include "humanpose_stream.h"
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
  std::vector<libvrm::vrm::HumanBones> m_humanBoneMap;
  std::vector<DirectX::XMFLOAT4> m_rotations;

  // constructor
  UdpNode(int id, std::string_view name)
    : GraphNodeBase(id, name)
  {
    m_scene = std::make_shared<libvrm::gltf::Scene>();

    // update preview
    m_scene->m_sceneUpdated.push_back(std::bind(
      &ScenePreview::OnSceneUpdated, &m_preview, std::placeholders::_1));

    m_udp = std::make_shared<UdpReceiver>();
    auto callback = [this](std::span<const uint8_t> data) {
      // udp update m_motion scene
      libvrm::srht::UpdateScene(m_scene, data);

      if (m_scene->m_roots.size()) {
        m_scene->m_roots[0]->CalcWorldMatrix(true);
        m_scene->RaiseSceneUpdated();

        // retarget human pose
        m_humanBoneMap.clear();
        m_rotations.clear();
        for (auto& node : m_scene->m_nodes) {
          if (auto humanoid = node->Humanoid) {
            m_humanBoneMap.push_back(humanoid->HumanBone);
            m_rotations.push_back(node->Transform.Rotation);
          }
        }

        auto& root = m_scene->m_roots[0]->Transform.Translation;
        Outputs[0].Value =
          libvrm::vrm::HumanPose{ .RootPosition = { root.x, root.y, root.z },
                                  .Bones = m_humanBoneMap,
                                  .Rotations = m_rotations };
      }
    };
    m_udp->Start(54345, callback);
  }

  void TimeUpdate(libvrm::Time time) override { m_udp->Update(); }

  void DrawContent() override
  {
    m_preview.Draw();
  }
};
}
