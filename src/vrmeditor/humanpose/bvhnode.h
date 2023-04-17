#pragma once
#include "app.h"
#include "humanpose_stream.h"
#include "scenepreview.h"
#include <vrm/bvh.h>
#include <vrm/bvhscene.h>
#include <vrm/humanpose.h>
#include <vrm/timeline.h>

namespace humanpose {
struct BvhNode : public GraphNodeBase
{
  std::shared_ptr<libvrm::gltf::Scene> m_scene;
  std::shared_ptr<libvrm::bvh::Bvh> m_bvh;

  ScenePreview m_preview;

  libvrm::Time m_lastTime = {};
  std::vector<libvrm::vrm::HumanBones> m_humanBoneMap;
  std::vector<DirectX::XMFLOAT4> m_rotations;

  // constructor
  BvhNode(int id, std::string_view name)
    : GraphNodeBase(id, name)
  {
    m_scene = std::make_shared<libvrm::gltf::Scene>();

    // update preview
    m_scene->m_sceneUpdated.push_back(std::bind(
      &ScenePreview::OnSceneUpdated, &m_preview, std::placeholders::_1));
  }

  void SetBvh(const std::shared_ptr<libvrm::bvh::Bvh>& bvh,
              const std::shared_ptr<libvrm::vrm::HumanBoneMap>& map)
  {
    m_bvh = bvh;
    libvrm::bvh::InitializeSceneFromBvh(m_scene, bvh);

    if (map) {
      // assign human bone
      for (auto& node : m_scene->m_nodes) {
        auto found = map->NameBoneMap.find(node->Name);
        if (found != map->NameBoneMap.end()) {
          node->Humanoid = libvrm::gltf::NodeHumanoidInfo{
            .HumanBone = found->second,
          };
        }
      }

    } else {
      App::Instance().Log(LogLevel::Wran) << "humanoid map not found";
    }
  }

  void TimeUpdate(libvrm::Time time) override
  {
    if (m_scene->m_roots.empty()) {
      return;
    }

    // update scene from bvh
    libvrm::bvh::UpdateSceneFromBvhFrame(m_scene, m_bvh, time);
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

  void DrawContent() override { m_preview.Draw(); }
};
}
