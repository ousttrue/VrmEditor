#include "mesh_gui.h"
#include <grapho/imgui/printfbuffer.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <vrm/gltfroot.h>

struct MeshGuiImpl
{
  int m_selected = 0;
  std::shared_ptr<libvrm::GltfRoot> m_root;
  grapho::imgui::PrintfBuffer m_buf;

  void SetGltf(const std::shared_ptr<libvrm::GltfRoot>& root) { m_root = root; }

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
            m_selected = i;
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
MeshGui::SetGltf(const std::shared_ptr<libvrm::GltfRoot>& root)
{
  m_impl->SetGltf(root);
}
