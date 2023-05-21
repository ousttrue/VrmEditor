#include "bvhnode.h"
#include "app.h"
#include "glr/scene_preview.h"
#include "humanpose_stream.h"
#include <grapho/orbitview.h>
#include <vrm/animation/runtime_scene.h>
#include <vrm/animation/timeline.h>
#include <vrm/bvh/bvhscene.h>
#include <vrm/humanoid/humanpose.h>

namespace humanpose {

// constructor
BvhNode::BvhNode(int id, std::string_view name)
  : GraphNodeBase(id, name)
{
  auto table = std::make_shared<libvrm::gltf::GltfRoot>();
  m_scene = std::make_shared<runtimescene::RuntimeScene>(table);

  // update preview
  m_preview = std::make_shared<glr::ScenePreview>(m_scene);
}

void
BvhNode::SetBvh(const std::shared_ptr<libvrm::bvh::Bvh>& bvh,
                const std::shared_ptr<libvrm::vrm::HumanBoneMap>& map)
{
  m_bvh = bvh;

  if (map) {

  } else {
    App::Instance().Log(LogLevel::Wran) << "humanoid map not found";
  }

  libvrm::bvh::InitializeSceneFromBvh(m_scene->m_table, bvh, map);
  m_scene->Reset();
}

void
BvhNode::TimeUpdate(libvrm::Time time)
{
  if (m_initialPose) {
    Outputs[0].Value = libvrm::vrm::HumanPose::Initial();
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
    m_scene->m_table->m_title.c_str(), color, sc.x, sc.y, 300, 300);
}

}
