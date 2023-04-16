#pragma once
#include "cuber.h"
#include "docks/gl3renderer.h"
#include "docks/rendertarget.h"
#include "humanpose_stream.h"
#include "udp_receiver.h"
#include <vrm/scene.h>
#include <vrm/srht_update.h>

struct UdpNode
  : public GraphNodeBase
  , IValue<libvrm::vrm::HumanPose>
{
  std::shared_ptr<libvrm::gltf::Scene> m_scene;
  std::shared_ptr<UdpReceiver> m_udp;
  std::shared_ptr<Cuber> m_cuber;
  std::shared_ptr<RenderTarget> m_rt;

  libvrm::Time m_lastTime = {};
  std::vector<libvrm::vrm::HumanBones> m_humanBoneMap;
  std::vector<DirectX::XMFLOAT4> m_rotations;
  libvrm::vrm::HumanPose m_pose;

  libvrm::vrm::HumanPose operator()() const override { return m_pose; }

  // constructor
  UdpNode(int id, std::string_view name)
    : GraphNodeBase(id, name)
  {
    m_scene = std::make_shared<libvrm::gltf::Scene>();

    // bind motion to scene
    m_scene->m_sceneUpdated.push_back([this](const libvrm::gltf::Scene& scene) {
      m_cuber->Instances.clear();
      if (scene.m_roots.size()) {
        scene.m_roots[0]->UpdateShapeInstanceRecursive(
          DirectX::XMMatrixIdentity(), m_cuber->Instances);
      }
    });

    m_udp = std::make_shared<UdpReceiver>();

    auto callback = [this](std::span<const uint8_t> data) {
      // udp update m_motion scene
      libvrm::srht::UpdateScene(m_scene, m_cuber->Instances, data);

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
        m_pose = { .RootPosition = { root.x, root.y, root.z },
                   .Bones = m_humanBoneMap,
                   .Rotations = m_rotations };
      }
    };

    m_udp->Start(54345, callback);

    // render target
    m_cuber = std::make_shared<Cuber>();
    m_rt =
      std::make_shared<RenderTarget>(std::make_shared<grapho::OrbitView>());
    m_rt->color[0] = 0.4f;
    m_rt->color[1] = 0.2f;
    m_rt->color[2] = 0.2f;
    m_rt->color[3] = 1.0f;

    m_rt->render = [this](const ViewProjection& camera) {
      Gl3Renderer::ClearRendertarget(camera);
      m_cuber->Render(camera);
    };
  }

  void TimeUpdate(libvrm::Time time) override { m_udp->Update(); }

  void DrawContent() override
  {
    auto pos = ImGui::GetCursorPos();
    auto size = ImVec2{ 300, 300 };
    m_rt->show_fbo(pos.x, pos.y, size.x, size.y);
  }
};
