#include "bvhnode.h"
#include "app.h"
#include "glr/cuber_preview.h"
#include "humanpose_stream.h"
#include <grapho/orbitview.h>
#include <vrm/bvhscene.h>
#include <vrm/humanpose.h>
#include <vrm/timeline.h>

namespace humanpose {

// constructor
BvhNode::BvhNode(int id, std::string_view name)
  : GraphNodeBase(id, name)
{
  m_scene = std::make_shared<libvrm::gltf::Scene>();

  // update preview
  m_preview = std::make_shared<glr::CuberPreview>();
  m_scene->m_sceneUpdated.push_back(
    std::bind(&glr::CuberPreview::OnSceneUpdated,
              m_preview.get(),
              std::placeholders::_1));
}

void
BvhNode::SetBvh(const std::shared_ptr<libvrm::bvh::Bvh>& bvh,
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

void
BvhNode::TimeUpdate(libvrm::Time time)
{
  if (m_initialPose) {
    // Outputs[0].Value = libvrm::vrm::HumanPose::Initial();
    m_scene->SetInitialPose();
  } else {
    // update scene from bvh
    libvrm::bvh::UpdateSceneFromBvhFrame(m_scene, m_bvh, time);
  }

  Outputs[0].Value = m_scene->UpdateHumanPose();
}

void
BvhNode::DrawContent()
{
  ImGui::Checkbox("init pose", &m_initialPose);
  m_preview->Draw();
}

}
