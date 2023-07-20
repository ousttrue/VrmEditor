#include <GL/glew.h>

#include "cuber.h"
#include "gl3renderer.h"
#include "line_gizmo.h"
#include "rendering_env.h"
#include "rendertarget.h"
#include "scene_renderer.h"
#include <DirectXCollision.h>
#include <boneskin/skinning_manager.h>
#include <limits>
#include <recti.h>
#include <recti_imgui.h>
#include <vrm/gltfroot.h>
#include <vrm/runtime_node.h>
#include <vrm/runtime_scene.h>

//   7+-+6
//   / /|
// 3+-+2 +5
// | |
// 0+-+1
DirectX::XMFLOAT3 p[8] = {
  { -0.5f, -0.5f, -0.5f }, //
  { +0.5f, -0.5f, -0.5f }, //
  { +0.5f, +0.5f, -0.5f }, //
  { -0.5f, +0.5f, -0.5f }, //
  { -0.5f, -0.5f, +0.5f }, //
  { +0.5f, -0.5f, +0.5f }, //
  { +0.5f, +0.5f, +0.5f }, //
  { -0.5f, +0.5f, +0.5f }, //
};

std::array<int, 4> triangles[] = {
  { 1, 5, 6, 2 }, // x+
  { 3, 2, 6, 7 }, // y+
  { 0, 1, 2, 3 }, // z+
  { 4, 7, 3, 0 }, // x-
  { 1, 0, 4, 5 }, // y-
  { 5, 6, 7, 4 }, // z-
};

static std::optional<float>
Intersect(DirectX::XMVECTOR origin,
          DirectX::XMVECTOR dir,
          DirectX::XMMATRIX m,
          int t)
{
  auto [i0, i1, i2, i3] = triangles[t];
  float dist;
  if (DirectX::TriangleTests::Intersects(
        origin,
        dir,
        DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&p[i0]), m),
        DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&p[i1]), m),
        DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&p[i2]), m),
        dist)) {
    return dist;
  } else if (DirectX::TriangleTests::Intersects(
               origin,
               dir,
               DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&p[i2]), m),
               DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&p[i3]), m),
               DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&p[i0]), m),
               dist)) {
    return dist;
  } else {
    return std::nullopt;
  }
}

namespace glr {

SceneRenderer::SceneRenderer(const std::shared_ptr<RenderingEnv>& env,
                             const std::shared_ptr<ViewSettings>& settings)
  : m_env(env ? env : std::make_shared<RenderingEnv>())
  , m_settings(settings ? settings : std::make_shared<ViewSettings>())
  , m_cuber(new Cuber)
  , m_gizmo(new LineGizmo)
  , m_screen(new recti::Screen)
  , m_camera(new grapho::camera::Camera)
{
}

static void
RenderScene(const grapho::camera::Camera& camera,
            const RenderingEnv& env,
            const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            std::span<const boneskin::NodeMesh> nodeMeshes,
            const std::shared_ptr<ViewSettings>& settings)
{
  if (nodeMeshes.size()) {
    glDisable(GL_DEPTH_TEST);
    if (settings->Skybox) {
      glr::RenderSkybox(camera.ProjectionMatrix, camera.ViewMatrix);
    }

    static std::vector<RenderPass> m_renderpass;
    m_renderpass.clear();
    glEnable(GL_DEPTH_TEST);
    if (settings->ShowMesh) {
      m_renderpass.push_back(RenderPass::Opaque);
    }
    if (settings->ShowShadow) {
      m_renderpass.push_back(RenderPass::ShadowMatrix);
    }
    if (settings->ShowMesh) {
      m_renderpass.push_back(RenderPass::Transparent);
    }

    glr::RenderPasses(m_renderpass, camera, env, root, bin, nodeMeshes);
  }
}

void
SceneRenderer::RenderStatic(const std::shared_ptr<libvrm::GltfRoot>& scene,
                            const grapho::camera::Viewport& viewport,
                            const grapho::camera::MouseState& mouse) const
{
  // update camera
  m_camera->Projection.SetViewport(viewport);
  m_camera->MouseInputTurntable(mouse);
  m_camera->Update();

  glr::ClearRendertarget(*m_camera, *m_env);

  auto nodestates = scene->NodeStates();
  auto nodeMeshes = boneskin::SkinningManager::Instance().ProcessSkin(
    *scene->m_gltf, scene->m_bin, nodestates);
  RenderScene(
    *m_camera, *m_env, *scene->m_gltf, scene->m_bin, nodeMeshes, m_settings);

  if (m_settings->ShowLine) {
    glr::RenderLine(*m_camera, m_gizmo->m_lines);
  }
  m_gizmo->Clear();

  if (m_settings->ShowCuber) {
    m_cuber->Instances.clear();

    for (auto m : scene->ShapeMatrices()) {
      m_cuber->Instances.push_back({
        .Matrix = m,
      });
    }

    m_cuber->Render(*m_camera);
  }

  // manipulator
  if (auto selected = scene->m_selected) {
    // TODO: conflict mouse event(left) with ImageButton
    DirectX::XMFLOAT4X4 m;
    DirectX::XMStoreFloat4x4(&m, selected->WorldInitialMatrix());

    auto op = recti::ROTATE;
    if (auto humanoid = selected->Humanoid) {
      if (*humanoid == libvrm::HumanBones::hips) {
        op |= recti::TRANSLATE;
      }
    } else {
      op |= recti::TRANSLATE;
      op |= recti::SCALE;
    }

    recti::Camera gizmo_camera{
      *((const recti::Mat4*)&m_camera->ViewMatrix),
      *((const recti::Mat4*)&m_camera->ProjectionMatrix),
      *((const recti::Vec4*)&m_camera->Projection.Viewport),
    };

    auto& io = ImGui::GetIO();
    recti::Mouse mouse{ io.MousePos, io.MouseDown[0] };

    m_screen->Begin(gizmo_camera, mouse);
    if (m_screen->Manipulate(
          (int64_t)selected.get(), op, recti::LOCAL, (float*)&m)) {
      // decompose feedback
      selected->SetWorldInitialMatrix(DirectX::XMLoadFloat4x4(&m));
      selected->CalcWorldInitialMatrix(true);

      scene->RaiseSceneUpdated();
    }
    recti::Render(m_screen->DrawList, ImGui::GetWindowDrawList());
  }
}

struct Hit
{
  uint32_t Index;
  float Distance;
};

void
SceneRenderer::RenderRuntime(
  const std::shared_ptr<libvrm::RuntimeScene>& runtime,
  const grapho::camera::Viewport& viewport,
  const grapho::camera::MouseState& mouse) const
{
  // update camera
  m_camera->Projection.SetViewport(viewport);
  m_camera->MouseInputTurntable(mouse);
  m_camera->Update();

  auto ray = m_camera->GetRay(mouse);

  glr::ClearRendertarget(*m_camera, *m_env);

  auto nodestates = runtime->m_base->NodeStates();
  if (nodestates.size()) {
    runtime->UpdateNodeStates(nodestates);
    m_settings->NextSpringDelta = {};
    auto nodeMeshes = boneskin::SkinningManager::Instance().ProcessSkin(
      *runtime->m_base->m_gltf, runtime->m_base->m_bin, nodestates);
    RenderScene(*m_camera,
                *m_env,
                *runtime->m_base->m_gltf,
                runtime->m_base->m_bin,
                nodeMeshes,
                m_settings);
  }

  if (m_settings->ShowLine) {
    runtime->DrawGizmo(m_gizmo.get());
    glr::RenderLine(*m_camera, m_gizmo->m_lines);
  }
  m_gizmo->Clear();

  auto distance = std::numeric_limits<float>::infinity();
  std::optional<uint32_t> closest;

  m_cuber->Instances.clear();

  auto matrices = runtime->ShapeMatrices();
  for (uint32_t i = 0; i < matrices.size(); ++i) {
    auto& m = matrices[i];
    m_cuber->Instances.push_back({
      .Matrix = m,
    });
    if (ray) {
      auto& cube = m_cuber->Instances.back();
      auto inv = DirectX::XMMatrixInverse(
        nullptr, DirectX::XMLoadFloat4x4(&cube.Matrix));
      auto local_ray = ray->Transform(inv);
      auto origin = DirectX::XMLoadFloat3(&ray->Origin);
      auto dir = DirectX::XMLoadFloat3(&ray->Direction);
      auto m = DirectX::XMLoadFloat4x4(&cube.Matrix);

      {
        float hit[3] = { 0, 0, 0 };

        if (auto d = Intersect(origin, dir, m, 0)) {
          cube.PositiveFaceFlag.x = 7;
          hit[0] = *d;
          if (*d < distance) {
            distance = *d;
            closest = i;
          }
        } else {
          cube.PositiveFaceFlag.x = 8;
        }

        if (auto d = Intersect(origin, dir, m, 1)) {
          cube.PositiveFaceFlag.y = 7;
          hit[1] = *d;
          if (*d < distance) {
            distance = *d;
            closest = i;
          }
        } else {
          cube.PositiveFaceFlag.y = 8;
        }

        if (auto d = Intersect(origin, dir, m, 2)) {
          cube.PositiveFaceFlag.z = 7;
          hit[2] = *d;
          if (*d < distance) {
            distance = *d;
            closest = i;
          }
        } else {
          cube.PositiveFaceFlag.z = 8;
        }
        // ImGui::InputFloat3(buf.Printf("%d.hit.positive", i), hit);
      }

      {
        float hit[3] = { 0, 0, 0 };
        if (auto d = Intersect(origin, dir, m, 3)) {
          cube.NegativeFaceFlag.x = 7;
          hit[0] = *d;
          if (*d < distance) {
            distance = *d;
            closest = i;
          }
        } else {
          cube.NegativeFaceFlag.x = 8;
        }

        if (auto d = Intersect(origin, dir, m, 4)) {
          cube.NegativeFaceFlag.y = 7;
          hit[1] = *d;
          if (*d < distance) {
            distance = *d;
            closest = i;
          }
        } else {
          cube.NegativeFaceFlag.y = 8;
        }

        if (auto d = Intersect(origin, dir, m, 5)) {
          cube.NegativeFaceFlag.z = 7;
          hit[2] = *d;
          if (*d < distance) {
            distance = *d;
            closest = i;
          }
        } else {
          cube.NegativeFaceFlag.z = 8;
        }
        // ImGui::InputFloat3(buf.Printf("%d.hit.negative", i), hit);
      }
    }
  }

  // manipulator
  bool manipulated = false;
  if (auto selected = runtime->GetSelectedNode()) {
    //   // TODO: conflict mouse event(left) with ImageButton
    DirectX::XMFLOAT4X4 m;
    DirectX::XMStoreFloat4x4(&m, selected->WorldMatrix());

    auto op = recti::ROTATE;
    if (auto humanoid = selected->Base->Humanoid) {
      if (*humanoid == libvrm::HumanBones::hips) {
        op |= recti::TRANSLATE;
      }
    } else {
      op |= recti::TRANSLATE;
      op |= recti::SCALE;
    }

    recti::Camera gizmo_camera{
      *((const recti::Mat4*)&m_camera->ViewMatrix),
      *((const recti::Mat4*)&m_camera->ProjectionMatrix),
      *((const recti::Vec4*)&m_camera->Projection.Viewport),
    };

    auto& io = ImGui::GetIO();
    recti::Mouse mouse{ io.MousePos, io.MouseDown[0] };

    m_screen->Begin(gizmo_camera, mouse);
    manipulated = m_screen->Manipulate(
      (int64_t)selected.get(), op, recti::LOCAL, (float*)&m);
    if (manipulated) {
      // decompose feedback
      selected->SetWorldMatrix(DirectX::XMLoadFloat4x4(&m));
      selected->CalcWorldMatrix(true);
    }

    // const float cubes[]{
    //   1, 0, 0, 0, //
    //   0, 1, 0, 0, //
    //   0, 0, 1, 0, //
    //   0, 0, 0, 1, //
    // };
    // auto cubes = runtime->ShapeMatrices();
    // m_screen->DrawCubes((const float*)cubes.data(), cubes.size());

    recti::Render(m_screen->DrawList, ImGui::GetWindowDrawList());
  }

  if (!manipulated && mouse.LeftDown) {
    if (closest) {
      runtime->SelectNode(runtime->m_nodes[*closest]);
    } else {
      // runtime->SelectNode(nullptr);
    }
  }

  if (m_settings->ShowCuber) {
    m_cuber->Render(*m_camera);
  }
}

} // namespace
