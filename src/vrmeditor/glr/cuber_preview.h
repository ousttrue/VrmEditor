#pragma once
#include "cuber.h"
#include "gl3renderer.h"
#include "overlay.h"
#include "rendertarget.h"
#include <imgui.h>
#include <vrm/scene.h>

namespace glr {
struct CuberPreview
{
  std::shared_ptr<Cuber> m_cuber;
  std::shared_ptr<RenderTarget> m_rt;

  CuberPreview()
  {
    // render target
    m_cuber = std::make_shared<Cuber>();
    m_rt =
      std::make_shared<RenderTarget>(std::make_shared<grapho::OrbitView>());
    m_rt->color[0] = 0.4f;
    m_rt->color[1] = 0.2f;
    m_rt->color[2] = 0.2f;
    m_rt->color[3] = 1.0f;

    m_rt->render = [this](const RenderingEnv& camera) {
      glr::ClearRendertarget(camera);
      m_cuber->Render(camera);
    };
  }

  void OnSceneUpdated(const libvrm::gltf::Scene& scene)
  {
    m_cuber->Instances.clear();
    if (scene.m_roots.size()) {
      scene.m_roots[0]->UpdateShapeInstanceRecursive(
        DirectX::XMMatrixIdentity(), m_cuber->Instances);
    }
  }

  void Draw(const char* title)
  {
    auto sc = ImGui::GetCursorScreenPos();
    auto pos = ImGui::GetCursorPos();

    auto size = ImVec2{ 300, 300 };
    m_rt->show_fbo(pos.x, pos.y, size.x, size.y);

    Overlay({ sc.x + size.x - 10, sc.y + 10 }, title);
  }
};
}
