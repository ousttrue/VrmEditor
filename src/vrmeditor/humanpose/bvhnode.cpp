#include "bvhnode.h"
#include "../view/scene_preview.h"
#include "app.h"
#include "humanpose_stream.h"
#include <plog/Log.h>
#include <vrm/bvh/bvhscene.h>
#include <vrm/humanoid/humanpose.h>
#include <vrm/runtime_scene.h>
#include <vrm/timeline.h>

namespace humanpose {

// constructor
BvhNode::BvhNode(int id, std::string_view name)
  : GraphNodeBase(id, name)
{
  auto table = std::make_shared<libvrm::GltfRoot>();
  m_scene = std::make_shared<libvrm::RuntimeScene>(table);

  m_preview = std::make_shared<ScenePreview>();
  m_preview->Settings()->ShowCuber = true;
  m_preview->SetRuntime(m_scene, {});
}

void
BvhNode::SetBvh(const std::shared_ptr<libvrm::bvh::Bvh>& bvh,
                const std::shared_ptr<libvrm::HumanBoneMap>& map)
{
  m_bvh = bvh;

  if (map) {

  } else {
    PLOG_WARNING << "humanoid map not found";
  }

  libvrm::bvh::InitializeSceneFromBvh(m_scene->m_base, bvh, map);
  m_scene->Reset();
}

void
BvhNode::TimeUpdate(libvrm::Time time)
{
  if (m_initialPose) {
    Outputs[0].Value = libvrm::HumanPose::Initial();
    // m_scene->SetInitialPose();
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
  auto sc = ImGui::GetCursorScreenPos();

  static float color[] = {
    0.2f,
    0.2f,
    0.2f,
    1.0f,
  };

  m_preview->ShowScreenRect(
    m_scene->m_base->m_title.c_str(), color, sc.x, sc.y, 300, 300);
}

}
