#include "bvhnode.h"
#include "app.h"
#include "glr/scene_preview.h"
#include "humanpose_stream.h"
#include <grapho/orbitview.h>
#include <vrm/bvhscene.h>
#include <vrm/humanpose.h>
#include <vrm/runtimescene/scene.h>
#include <vrm/timeline.h>

namespace humanpose {

// constructor
BvhNode::BvhNode(int id, std::string_view name)
  : GraphNodeBase(id, name)
{
  auto table = std::make_shared<libvrm::gltf::Scene>();
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
}

void
BvhNode::TimeUpdate(libvrm::Time time)
{
  if (m_initialPose) {
    // Outputs[0].Value = libvrm::vrm::HumanPose::Initial();
    m_scene->m_table->SetInitialPose();
  } else {
    // update scene from bvh
    libvrm::bvh::UpdateSceneFromBvhFrame(m_scene->m_table, m_bvh, time);
  }

  Outputs[0].Value = m_scene->m_table->UpdateHumanPose();
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
