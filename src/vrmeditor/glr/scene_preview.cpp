#include "scene_preview.h"
#include "cuber.h"
#include "gl3renderer.h"
#include "line_gizmo.h"
#include "overlay.h"
#include "rendertarget.h"
#include <DirectXMath.h>
#include <ImGuizmo.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <vrm/gizmo.h>

namespace glr {
ScenePreview::ScenePreview(
  const std::shared_ptr<libvrm::gltf::Scene>& scene,
  const std::shared_ptr<grapho::OrbitView>& view,
  const std::shared_ptr<libvrm::gltf::SceneContext>& context)
  : m_rt(std::make_shared<RenderTarget>(view))
  , m_cuber(std::make_shared<Cuber>())
{
  m_rt->color[0] = 0.2f;
  m_rt->color[1] = 0.2f;
  m_rt->color[2] = 0.2f;
  m_rt->color[3] = 1.0f;

  m_rt->render =
    [this, scene, selection = context, gizmo = std::make_shared<LineGizmo>()](
      const RenderingEnv& env) {
      glr::ClearRendertarget(env);

      libvrm::gltf::RenderFunc render =
        [this, &env](const std::shared_ptr<libvrm::gltf::Mesh>& mesh,
                     const libvrm::gltf::MeshInstance& meshInstance,
                     const float m[16]) {
          if (m_showMesh) {
            glr::Render(RenderPass::Color, env, mesh, meshInstance, m);
          }
          if (m_showShadow) {
            glr::Render(RenderPass::ShadowMatrix, env, mesh, meshInstance, m);
          }
        };

      scene->Render(render, gizmo.get());
      if (m_showLine) {
        glr::RenderLine(env, gizmo->m_lines);
      }
      gizmo->Clear();

      if (m_showCuber) {
        m_cuber->Instances.clear();
        static_assert(sizeof(cuber::Instance) == sizeof(libvrm::gltf::Instance),
                      "Instance size");
        for (auto& root : scene->m_roots) {
          root->UpdateShapeInstanceRecursive(
            DirectX::XMMatrixIdentity(),
            [cuber = m_cuber](const libvrm::gltf::Instance& instance) {
              cuber->Instances.push_back(*((const cuber::Instance*)&instance));
            });
        }
        m_cuber->Render(env);
      }

      // gizmo
      if (auto node = selection->selected.lock()) {
        // TODO: conflict mouse event(left) with ImageButton
        DirectX::XMFLOAT4X4 m;
        DirectX::XMStoreFloat4x4(&m, node->WorldMatrix());
        ImGuizmo::GetContext().mAllowActiveHoverItem = true;
        if (ImGuizmo::Manipulate(env.ViewMatrix,
                                 env.ProjectionMatrix,
                                 ImGuizmo::TRANSLATE | ImGuizmo::ROTATE,
                                 ImGuizmo::LOCAL,
                                 (float*)&m,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr)) {
          // decompose feedback
          node->SetWorldMatrix(DirectX::XMLoadFloat4x4(&m));
        }
        ImGuizmo::GetContext().mAllowActiveHoverItem = false;
      }
    };

  m_popup = [this]() {
    if (ImGui::BeginPopupContextItem(m_popupName.c_str())) {
      ImGui::Checkbox("mesh", &m_showMesh);
      ImGui::Checkbox("shadow", &m_showShadow);
      ImGui::Checkbox("line", &m_showLine);
      ImGui::Checkbox("bone", &m_showCuber);
      ImGui::EndPopup();
    }
  };
}

void
ScenePreview::ShowScreenRect(const char* title,
                             float x,
                             float y,
                             float w,
                             float h)
{
  auto sc = ImGui::GetCursorScreenPos();
  m_rt->show_fbo(x, y, w, h);
  // top, right pivot
  Overlay({ sc.x + w - 10, sc.y + 10 }, title, m_popupName.c_str(), m_popup);
}

void
ScenePreview::ShowFullWindow(const char* title)
{
  auto pos = ImGui::GetWindowPos();
  pos.y += ImGui::GetFrameHeight();
  auto size = ImGui::GetContentRegionAvail();
  ShowScreenRect(title, pos.x, pos.y, size.x, size.y);
}

}
