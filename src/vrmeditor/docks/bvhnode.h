#pragma once
#include "app.h"
#include "cuber.h"
#include "gl3renderer.h"
#include "humanpose_stream.h"
#include "rendertarget.h"
#include <vrm/bvh.h>
#include <vrm/bvhscene.h>
#include <vrm/scene.h>

struct BvhNode : public GraphNodeBase
{
  std::shared_ptr<libvrm::gltf::Scene> m_scene;
  std::shared_ptr<libvrm::bvh::Bvh> m_bvh;
  std::shared_ptr<Cuber> m_cuber;
  std::shared_ptr<RenderTarget> m_rt;

  // constructor
  BvhNode(int id, std::string_view name)
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

  void SetBvh(const std::shared_ptr<libvrm::bvh::Bvh>& bvh,
              const std::shared_ptr<libvrm::vrm::HumanBoneMap>& map)
  {
    m_bvh = bvh;
    libvrm::bvh::InitializeSceneFromBvh(m_scene, bvh);
    m_scene->m_roots[0]->UpdateShapeInstanceRecursive(
      DirectX::XMMatrixIdentity(), m_cuber->Instances);

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

  void Update(libvrm::Time time) override
  {
    if (m_scene->m_roots.size()) {
      libvrm::bvh::UpdateSceneFromBvhFrame(m_scene, m_bvh, time);
      m_scene->m_roots[0]->CalcWorldMatrix(true);
      m_scene->RaiseSceneUpdated();
    }
  }

  void DrawContent() override
  {
    auto pos = ImGui::GetCursorPos();
    auto size = ImVec2{ 300, 300 };
    m_rt->show_fbo(pos.x, pos.y, size.x, size.y);
  }
};
