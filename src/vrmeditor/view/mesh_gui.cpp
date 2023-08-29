#include "mesh_gui.h"
#include "im_fbo.h"
#include <boneskin/meshdeformer.h>
#include <glr/gizmo.h>
#include <glr/gl3renderer.h>
#include <glr/rendering_env.h>
#include <grapho/camera/camera.h>
#include <grapho/camera/viewport.h>
#include <grapho/imgui/printfbuffer.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <vrm/gltfroot.h>
#include <vrm/runtime_scene.h>

struct MeshGuiImpl
{
  // mesh selector
  int m_selected = -1;
  std::shared_ptr<libvrm::GltfRoot> m_root;
  grapho::imgui::PrintfBuffer m_buf;

  // view
  std::shared_ptr<glr::RenderingEnv> m_env;
  std::shared_ptr<glr::Gizmo> m_gizmo;
  std::shared_ptr<grapho::camera::Camera> m_camera;
  std::shared_ptr<libvrm::RuntimeScene> m_runtime;
  std::shared_ptr<ImFbo> m_fbo;
  std::shared_ptr<boneskin::MeshDeformer> m_deformer;

  std::shared_ptr<boneskin::BaseMesh> m_baseMesh;
  std::unordered_map<uint32_t, float> m_morphMap;

  MeshGuiImpl()
  {
    m_fbo = ImFbo::Create();
    m_gizmo = std::make_shared<glr::Gizmo>();
    m_env = std::make_shared<glr::RenderingEnv>();
    m_env->ClearColor = { 0.3f, 0.3f, 0.3f, 1.0f };
    m_camera = std::make_shared<grapho::camera::Camera>();
  }

  void SetGltf(const std::shared_ptr<libvrm::GltfRoot>& root)
  {
    m_root = root;
    m_runtime = std::make_shared<libvrm::RuntimeScene>(m_root);
    m_deformer = std::make_shared<boneskin::MeshDeformer>();
  }

  void Select(int selected)
  {
    if (selected == m_selected) {
      return;
    }
    m_selected = selected;
    m_morphMap.clear();
    m_baseMesh = m_deformer->GetOrCreateBaseMesh(
      *m_root->m_gltf, m_root->m_bin, m_selected);
  }

  grapho::imgui::SplitterObject m_outer;
  grapho::imgui::SplitterObject m_inner;

  const int no_scroll =
    (ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

  void ShowGui()
  {
    if (!m_root) {
      return;
    }

    auto size = ImGui::GetContentRegionAvail();
    auto [o1, o2] = m_outer.SplitHorizontal(size, 0.3f);

    ImGui::BeginChild("1", ImVec2(-1, o1), true);
    _MeshSelector(o1);
    ImGui::EndChild();

    ImGui::BeginChild("2", ImVec2(-1, o2), true, no_scroll);
    {
      auto [i1, i2] = m_inner.SplitHorizontal({ -1, o2 });
      ImGui::BeginChild("21", ImVec2(-1, i1), true);

      if (ImGui::BeginTabBar("HierarchyTabs")) {
        if (ImGui::BeginTabItem("MorphTargets")) {
          _MorphList();
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Primitives")) {
          _PrimList();
          ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
      }

      ImGui::EndChild();

      ImGui::BeginChild("22", ImVec2(-1, i2), true, no_scroll);
      _View({ size.x, i2 });
      ImGui::EndChild();
    }
    ImGui::EndChild();
  }

  void _MeshSelector(float o1)
  {
    std::array<const char*, 3> cols = {
      "Index",
      "Name",
      "Prims",
    };

    if (grapho::imgui::BeginTableColumns("##MeshTable", cols, { 0, o1 })) {
      for (int i = 0; i < m_root->m_gltf->Meshes.size(); ++i) {
        auto mesh = m_root->m_gltf->Meshes[i];
        ImGui::TableNextRow();
        // 0
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(m_buf.Printf("%d", i));
        // 1
        ImGui::TableNextColumn();
        if (ImGui::Selectable((const char*)mesh.NameString().c_str(),
                              i == m_selected)) {
          Select(i);
        }
        // 2
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(m_buf.Printf("%d", mesh.Primitives.size()));
      }

      ImGui::EndTable();
    }
  }

  void _MorphList()
  {
    if (m_selected >= 0 || m_selected <= m_root->m_gltf->Meshes.size()) {
      auto mesh = m_root->m_gltf->Meshes[m_selected];
      int morph_targets = -1;
      for (int i = 0; i < mesh.Primitives.size(); ++i) {
        auto prim = mesh.Primitives[i];
        if (i == 0) {
          morph_targets = prim.Targets.size();
        } else {
          if (prim.Targets.size() != morph_targets) {
            // Invalid Error morph targets
            morph_targets = -1;
          }
        }
      }
      if (morph_targets < 0) {
        // Error
      } else if (morph_targets > 0) {
        // show sliders
        for (int i = 0; i < morph_targets; ++i) {
          auto found = m_morphMap.find(i);
          if (found == m_morphMap.end()) {
            found = m_morphMap.insert({ i, 0.0f }).first;
          }
          ImGui::SliderFloat(
            m_buf.Printf(
              "[%d] %s", i, m_baseMesh->m_morphTargets[i]->Name.c_str()),
            &found->second,
            0,
            1);
        }
      }
    }
  }

  void _PrimList()
  {
    if (m_selected >= 0 || m_selected <= m_root->m_gltf->Meshes.size()) {
      auto mesh = m_root->m_gltf->Meshes[m_selected];

      std::array<const char*, 4> cols = {
        "Index",
        "Vertices",
        "Indices",
        "MorphTargets",
      };

      if (grapho::imgui::BeginTableColumns("##PrimTable", cols, { 0, 0 })) {
        for (int i = 0; i < mesh.Primitives.size(); ++i) {
          auto prim = mesh.Primitives[i];
          ImGui::TableNextRow();
          // 0
          ImGui::TableNextColumn();
          ImGui::TextUnformatted(m_buf.Printf("%d", i));
          // 1
          ImGui::TableNextColumn();
          ImGui::TextUnformatted(
            m_buf.Printf("%u",
                         (uint32_t)*m_root->m_gltf
                           ->Accessors[*prim.Attributes()->POSITION_Id()]
                           .Count()));
          // 2
          ImGui::TableNextColumn();
          ImGui::TextUnformatted(m_buf.Printf(
            "%u",
            (uint32_t)*m_root->m_gltf->Accessors[*prim.IndicesId()].Count()));
          // 3
          ImGui::TableNextColumn();
          ImGui::TextUnformatted(m_buf.Printf("%zu", prim.Targets.size()));
        }
        ImGui::EndTable();
      }
    }
  }

  void _View(const ImVec2& size)
  {
    auto pos = ImGui::GetCursorScreenPos();
    grapho::camera::Viewport vp{ pos.x, pos.y, size.x, size.y };
    float color[4]{ 0, 0, 0, 1 };
    m_fbo->ShowFbo(vp,
                   color,
                   std::bind(&MeshGuiImpl::Render,
                             this,
                             std::placeholders::_1,
                             std::placeholders::_2));
  }

  void Render(const grapho::camera::Viewport& vp,
              const grapho::camera::MouseState& mouse)
  {
    // update camera
    m_camera->Projection.SetViewport(vp);
    m_camera->MouseInputTurntable(mouse);
    m_camera->Update();

    // clear
    glr::ClearRendertarget(*m_camera, *m_env);

    // grid
    m_gizmo->Render(*m_camera, false, true);

    // selected mesh
    if (m_selected >= 0 && m_selected < m_root->m_gltf->Meshes.size()) {
      auto mesh = m_root->m_gltf->Meshes[m_selected];
      DirectX::XMFLOAT4X4 identity = {
        1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
      };
      auto baseMesh = m_deformer->GetOrCreateBaseMesh(
        *m_root->m_gltf, m_root->m_bin, m_selected);

      auto deformed = m_deformer->GetOrCreateDeformedMesh(m_selected, baseMesh);

      deformed->ApplyMorphTarget(*baseMesh, m_morphMap);

      glr::RenderPass pass[] = { glr::RenderPass::Wireframe };
      glr::RenderPasses(pass,
                        *m_camera,
                        *m_env,
                        *m_root->m_gltf,
                        m_root->m_bin,
                        {
                          .MeshId = (uint32_t)m_selected,
                          .Matrix = identity,
                          .BaseMesh = baseMesh,
                          .Vertices = deformed->Vertices,
                        });
    }
  }
};

MeshGui::MeshGui()
  : m_impl(new MeshGuiImpl)
{
}

MeshGui::~MeshGui()
{
  delete m_impl;
}

void
MeshGui::ShowGui()
{
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  m_impl->ShowGui();
  ImGui::PopStyleVar();
}

void
MeshGui::SetGltf(const std::shared_ptr<libvrm::GltfRoot>& root)
{
  m_impl->SetGltf(root);
}
