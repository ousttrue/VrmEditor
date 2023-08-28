#include <GL/glew.h>

#include "gizmo.h"
#include "gl3renderer.h"
#include "rendering_env.h"
#include "rendertarget.h"
#include "scene_renderer.h"
#include <DirectXCollision.h>
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

inline DirectX::XMMATRIX
GetMatrix(const std::shared_ptr<libvrm::Node>& node)
{
  return node->WorldInitialMatrix();
}
inline DirectX::XMMATRIX
GetMatrix(const std::shared_ptr<libvrm::RuntimeNode>& node)
{
  return node->WorldMatrix();
}
inline void
SetMatrix(const std::shared_ptr<libvrm::Node>& node,
          const DirectX::XMFLOAT4X4& m)
{
  node->SetWorldInitialMatrix(DirectX::XMLoadFloat4x4(&m));
  node->CalcWorldInitialMatrix(true);
}
inline void
SetMatrix(const std::shared_ptr<libvrm::RuntimeNode>& node,
          const DirectX::XMFLOAT4X4& m)
{
  node->SetWorldMatrix(DirectX::XMLoadFloat4x4(&m));
  node->CalcWorldMatrix(true);
}

std::string
GetNodeLabel(const std::shared_ptr<libvrm::Node>& node)
{
  return node->Name;
}

std::string
GetNodeLabel(const std::shared_ptr<libvrm::RuntimeNode>& node)
{
  return GetNodeLabel(node->Base);
}

template<typename T>
static bool
ScreenGizmo(const grapho::camera::Camera& camera,
            const grapho::camera::MouseState& mouse,
            const std::shared_ptr<T>& scene,
            const std::shared_ptr<recti::Screen>& screen,
            std::optional<uint32_t> selectedIndex,
            std::optional<uint32_t> hoverIndex)
{
  recti::Camera gizmo_camera{
    *((const recti::Mat4*)&camera.ViewMatrix),
    *((const recti::Mat4*)&camera.ProjectionMatrix),
    *((const recti::Vec4*)&camera.Projection.Viewport),
  };
  auto& io = ImGui::GetIO();
  recti::Mouse gizmo_mouse{ io.MousePos, io.MouseDown[0] };
  bool active = false;
  screen->Begin(gizmo_camera, gizmo_mouse);
  {
    auto cubes = scene->ShapeMatrices();
    if (hoverIndex) {
      auto& m = cubes[*hoverIndex];
      screen->DrawCubes((const float*)&m, 1);
      screen->DrawText(m.m[3], GetNodeLabel(scene->m_nodes[*hoverIndex]));
    }
    // if (selectedIndex) {
    //   screen->DrawCubes((const float*)(cubes.data() + *selectedIndex), 1);
    // }

    if (auto selected = scene->GetSelectedNode()) {
      auto op = recti::ROTATE;
      if (auto humanoid = selected->GetHumanBone()) {
        if (*humanoid == libvrm::HumanBones::hips) {
          op |= recti::TRANSLATE;
        }
      } else {
        op |= recti::TRANSLATE;
        op |= recti::SCALE;
      }

      DirectX::XMFLOAT4X4 m;
      DirectX::XMStoreFloat4x4(&m, GetMatrix(selected));
      active = screen->Manipulate(
        (int64_t)selected.get(), op, recti::LOCAL, (float*)&m);
      if (active) {
        SetMatrix(selected, m);
      }
    }
  }
  recti::Render(screen->DrawList, ImGui::GetWindowDrawList());
  return active;
}

namespace glr {

SceneRenderer::SceneRenderer(const std::shared_ptr<RenderingEnv>& env,
                             const std::shared_ptr<ViewSettings>& settings)
  : m_env(env ? env : std::make_shared<RenderingEnv>())
  , m_settings(settings ? settings : std::make_shared<ViewSettings>())
  , m_gizmo(new Gizmo)
  , m_screen(new recti::Screen)
  , m_camera(new grapho::camera::Camera)
{
}

void
SceneRenderer::RenderScene(const grapho::camera::Camera& camera,
                           const RenderingEnv& env,
                           const gltfjson::Root& root,
                           const gltfjson::Bin& bin,
                           std::span<const boneskin::NodeMesh> nodeMeshes,
                           const ViewSettings& settings)
{
  if (nodeMeshes.size()) {
    glDisable(GL_DEPTH_TEST);
    if (settings.Skybox) {
      glr::RenderSkybox(camera.ProjectionMatrix, camera.ViewMatrix);
    }

    static std::vector<RenderPass> m_renderpass;
    m_renderpass.clear();
    glEnable(GL_DEPTH_TEST);
    if (settings.ShowMesh) {
      m_renderpass.push_back(RenderPass::Opaque);
    }
    if (settings.ShowShadow) {
      m_renderpass.push_back(RenderPass::ShadowMatrix);
    }
    if (settings.ShowMesh) {
      m_renderpass.push_back(RenderPass::Transparent);
    }

    glr::RenderPasses(m_renderpass, camera, env, root, bin, nodeMeshes);
  }
}

void
SceneRenderer::RenderFrame(grapho::camera::Camera& camera,
                           const RenderingEnv& env,
                           const gltfjson::Root& gltf,
                           const gltfjson::Bin& bin,
                           std::span<boneskin::NodeState> nodestates,
                           const ViewSettings& settings,
                           const std::shared_ptr<Gizmo>& gizmo,
                           std::span<const DirectX::XMFLOAT4X4> matrices,
                           std::optional<uint32_t> selected,
                           std::optional<uint32_t> hover)
{
  glr::ClearRendertarget(camera, env);

  if (nodestates.size()) {
    auto nodeMeshes =
      boneskin::SkinningManager::Instance().ProcessSkin(gltf, bin, nodestates);
    RenderScene(camera, env, gltf, bin, nodeMeshes, settings);
  }

  const int SELECTED = 9;
  const int HOVER = 10;
  gizmo->Instances.clear();
  for (uint32_t i = 0; i < matrices.size(); ++i) {
    auto& m = matrices[i];
    gizmo->Instances.push_back({
      .Matrix = m,
    });
    if (selected && i == *selected) {
      gizmo->Instances.back().PositiveFaceFlag.x = SELECTED;
      gizmo->Instances.back().PositiveFaceFlag.y = SELECTED;
      gizmo->Instances.back().PositiveFaceFlag.z = SELECTED;
      gizmo->Instances.back().NegativeFaceFlag.x = SELECTED;
      gizmo->Instances.back().NegativeFaceFlag.y = SELECTED;
      gizmo->Instances.back().NegativeFaceFlag.z = SELECTED;
    } else if (hover && i == *hover) {
      gizmo->Instances.back().PositiveFaceFlag.x = HOVER;
      gizmo->Instances.back().PositiveFaceFlag.y = HOVER;
      gizmo->Instances.back().PositiveFaceFlag.z = HOVER;
      gizmo->Instances.back().NegativeFaceFlag.x = HOVER;
      gizmo->Instances.back().NegativeFaceFlag.y = HOVER;
      gizmo->Instances.back().NegativeFaceFlag.z = HOVER;
    }
  }
  gizmo->Render(camera, settings.ShowCuber, settings.ShowLine);
  gizmo->Clear();
}

template<typename T>
static void
RenderFrame(grapho::camera::Camera& camera,
            const grapho::camera::Viewport& viewport,
            const grapho::camera::MouseState& mouse,
            const RenderingEnv& env,
            const gltfjson::Root& gltf,
            const gltfjson::Bin& bin,
            std::span<boneskin::NodeState> nodestates,
            const ViewSettings& settings,
            const std::shared_ptr<Gizmo>& gizmo,
            std::span<const DirectX::XMFLOAT4X4> matrices,
            const std::shared_ptr<T>& scene,
            const std::shared_ptr<recti::Screen>& screen,
            std::optional<uint32_t> selected,
            std::optional<uint32_t> hover)
{
  SceneRenderer::RenderFrame(camera,
                             env,
                             gltf,
                             bin,
                             nodestates,
                             settings,
                             gizmo,
                             matrices,
                             selected,
                             hover);
  // manipulator
  auto manipulated = ScreenGizmo(camera, mouse, scene, screen, selected, hover);
  if (!manipulated && camera.InViewport(mouse) && mouse.LeftDown) {
    if (hover) {
      scene->SelectNode(scene->m_nodes[*hover]);
    } else {
      // runtime->SelectNode(nullptr);
    }
  }
}

static std::optional<uint32_t>
Hover(const grapho::camera::Camera& camera,
      const grapho::camera::MouseState& mouse,
      std::span<const DirectX::XMFLOAT4X4> matrices)
{
  auto ray = camera.GetRay(mouse);
  if (!ray) {
    return std::nullopt;
  }

  auto distance = std::numeric_limits<float>::infinity();
  std::optional<uint32_t> closest;
  for (uint32_t i = 0; i < matrices.size(); ++i) {
    auto m = DirectX::XMLoadFloat4x4(&matrices[i]);

    // auto& cube = cuber->Instances.back();
    // auto inv = DirectX::XMMatrixInverse(
    //   nullptr, DirectX::XMLoadFloat4x4(&cube.Matrix));
    // auto local_ray = ray->Transform(inv);
    auto origin = DirectX::XMLoadFloat3(&ray->Origin);
    auto dir = DirectX::XMLoadFloat3(&ray->Direction);
    // auto m = DirectX::XMLoadFloat4x4(&cube.Matrix);

    {
      float hit[3] = { 0, 0, 0 };

      if (auto d = Intersect(origin, dir, m, 0)) {
        // cube.PositiveFaceFlag.x = 7;
        hit[0] = *d;
        if (*d < distance) {
          distance = *d;
          closest = i;
        }
      } else {
        // cube.PositiveFaceFlag.x = 8;
      }

      if (auto d = Intersect(origin, dir, m, 1)) {
        // cube.PositiveFaceFlag.y = 7;
        hit[1] = *d;
        if (*d < distance) {
          distance = *d;
          closest = i;
        }
      } else {
        // cube.PositiveFaceFlag.y = 8;
      }

      if (auto d = Intersect(origin, dir, m, 2)) {
        // cube.PositiveFaceFlag.z = 7;
        hit[2] = *d;
        if (*d < distance) {
          distance = *d;
          closest = i;
        }
      } else {
        // cube.PositiveFaceFlag.z = 8;
      }
      // ImGui::InputFloat3(buf.Printf("%d.hit.positive", i), hit);
    }

    {
      float hit[3] = { 0, 0, 0 };
      if (auto d = Intersect(origin, dir, m, 3)) {
        // cube.NegativeFaceFlag.x = 7;
        hit[0] = *d;
        if (*d < distance) {
          distance = *d;
          closest = i;
        }
      } else {
        // cube.NegativeFaceFlag.x = 8;
      }

      if (auto d = Intersect(origin, dir, m, 4)) {
        // cube.NegativeFaceFlag.y = 7;
        hit[1] = *d;
        if (*d < distance) {
          distance = *d;
          closest = i;
        }
      } else {
        // cube.NegativeFaceFlag.y = 8;
      }

      if (auto d = Intersect(origin, dir, m, 5)) {
        // cube.NegativeFaceFlag.z = 7;
        hit[2] = *d;
        if (*d < distance) {
          distance = *d;
          closest = i;
        }
      } else {
        // cube.NegativeFaceFlag.z = 8;
      }
      // ImGui::InputFloat3(buf.Printf("%d.hit.negative", i), hit);
    }
  }
  return closest;
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

  auto hover = Hover(*m_camera, mouse, scene->ShapeMatrices());

  auto nodestates = scene->NodeStates();
  ::glr::RenderFrame(*m_camera,
                     viewport,
                     mouse,
                     *m_env,
                     *scene->m_gltf,
                     scene->m_bin,
                     nodestates,
                     *m_settings,
                     m_gizmo,
                     scene->ShapeMatrices(),
                     scene,
                     m_screen,
                     scene->SelectedIndex(),
                     hover);
}

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

  // node hover
  auto hover = Hover(*m_camera, mouse, runtime->ShapeMatrices());

  auto nodestates = runtime->m_base->NodeStates();
  if (nodestates.size()) {
    runtime->UpdateNodeStates(nodestates);
    m_settings->NextSpringDelta = {};
  }

  if (m_settings->ShowSpring) {
    runtime->DrawGizmo(m_gizmo.get());
  }

  // render scene
  ::glr::RenderFrame(*m_camera,
                     viewport,
                     mouse,
                     *m_env,
                     *runtime->m_base->m_gltf,
                     runtime->m_base->m_bin,
                     nodestates,
                     *m_settings,
                     m_gizmo,
                     runtime->ShapeMatrices(),
                     runtime,
                     m_screen,
                     runtime->m_base->SelectedIndex(),
                     hover);
}

} // namespace
