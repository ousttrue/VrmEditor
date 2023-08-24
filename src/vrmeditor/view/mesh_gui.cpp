#include "mesh_gui.h"
#include "im_fbo.h"
#include <glr/scene_renderer.h>
#include <grapho/imgui/printfbuffer.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <vrm/gltfroot.h>
#include <vrm/runtime_scene.h>

struct MeshGuiImpl
{
  // seector
  int m_selected = -1;
  std::shared_ptr<libvrm::GltfRoot> m_root;
  grapho::imgui::PrintfBuffer m_buf;

  // view
  std::shared_ptr<glr::RenderingEnv> m_env;
  std::shared_ptr<glr::ViewSettings> m_settings =
    std::make_shared<glr::ViewSettings>();
  std::shared_ptr<glr::SceneRenderer> m_renderer;
  std::shared_ptr<libvrm::RuntimeScene> m_runtime;
  std::shared_ptr<ImFbo> m_fbo;

  MeshGuiImpl()
  {
    m_renderer = std::make_shared<glr::SceneRenderer>(m_env, m_settings);
    m_fbo = ImFbo::Create();
  }

  void SetGltf(const std::shared_ptr<libvrm::GltfRoot>& root)
  {
    m_root = root;
    m_runtime = std::make_shared<libvrm::RuntimeScene>(m_root);
  }

  void Select(int selected)
  {
    if (selected == m_selected) {
      return;
    }
    m_selected = selected;

    // create runtime scene that has a Node with selected mesh
    // m_runtime = std::make_shared<libvrm::RuntimeScene>(m_root);
  }

  void ShowGui()
  {
    if (!m_root) {
      return;
    }

    {
      std::array<const char*, 3> cols = {
        "Index",
        "Name",
        "Prims",
      };

      if (grapho::imgui::BeginTableColumns("##MeshTable", cols, { 0, 200 })) {
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

    if (m_selected < 0 || m_selected >= m_root->m_gltf->Meshes.size()) {
      return;
    }

    {
      auto mesh = m_root->m_gltf->Meshes[m_selected];

      std::array<const char*, 4> cols = {
        "Index",
        "Vertices",
        "Indices",
        "MorphTargets",
      };

      int morph_targets = -1;
      if (grapho::imgui::BeginTableColumns("##PrimTable", cols, { 0, 200 })) {
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
          if (i == 0) {
            morph_targets = prim.Targets.size();
          } else {
            if (prim.Targets.size() != morph_targets) {
              // Invalid Error morph targets
              morph_targets = -1;
            }
          }
        }
        ImGui::EndTable();
      }

      if (morph_targets < 0) {
        // Error
      } else if (morph_targets > 0) {
        // show sliders
        for (int i = 0; i < morph_targets; ++i) {
          float value = 0;
          ImGui::SliderFloat(m_buf.Printf("morph %d", i), &value, 0, 1);
        }
      }
    }

    // 3D View gray scale
    // MorphTarget
  }

  void ShowView()
  {
    if (!m_runtime) {
      return;
    }

    auto pos = ImGui::GetCursorScreenPos();
    auto size = ImGui::GetContentRegionAvail();
    // ShowScreenRect(title, color, pos.x, pos.y, size.x, size.y);

    auto sc = ImGui::GetCursorScreenPos();
    grapho::camera::Viewport vp{ pos.x, pos.y, size.x, size.y };
    float color[4]{ 0, 0, 0, 1 };
    m_fbo->ShowFbo(vp, color, [=](const auto& vp, const auto& mouse) {
      m_renderer->RenderRuntime(m_runtime, vp, mouse);
    });
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
  m_impl->ShowGui();
}

void
MeshGui::ShowView()
{
  m_impl->ShowView();
}

void
MeshGui::SetGltf(const std::shared_ptr<libvrm::GltfRoot>& root)
{
  m_impl->SetGltf(root);
}
