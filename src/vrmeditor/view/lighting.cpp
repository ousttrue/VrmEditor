#include <GL/glew.h>

#include "lighting.h"
#include <gltfjson.h>

#include <imgui.h>

#include <glr/gl3renderer.h>
#include <grapho/gl3/texture.h>
#include <grapho/imgui/widgets.h>

struct LightingImpl
{
  std::shared_ptr<libvrm::GltfRoot> m_root;
  std::shared_ptr<grapho::gl3::Texture> m_hdr;

  bool LoadHdr(const std::filesystem::path& path)
  {
    m_hdr = glr::LoadPbr_LOGL(path);
    return m_hdr != nullptr;
  }

  void SetGltf(const std::shared_ptr<libvrm::GltfRoot>& root) { m_root = root; }

  void ShowGui()
  {
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader("IBR")) {
      if (m_hdr) {
        ImGui::Image((ImTextureID)(intptr_t)m_hdr->Handle(), { 300, 300 });
      }
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader("KHR_lights_punctual")) {
      if (!m_root) {
        ImGui::TextUnformatted("no lights");
      } else if (auto KHR_lights_punctual =
                   m_root->m_gltf
                     ->GetExtension<gltfjson::KHR_lights_punctual>()) {

        std::array<const char*, 4> cols = {
          "name", "type", "color", "intensity"
        };

        if (grapho::imgui::BeginTableColumns("##_lights", cols)) {
          for (auto light : KHR_lights_punctual->Lights) {
            ImGui::PushID(light.m_json.get());
            ImGui::TableNextRow();

            // 0
            ImGui::TableNextColumn();
            ImGui::TextUnformatted((const char*)light.Name().c_str());

            // 1
            ImGui::TableNextColumn();
            ImGui::TextUnformatted((const char*)light.Type().c_str());

            // 2
            ImGui::TableNextColumn();
            if (auto color = light.Color()) {
              ImGui::SetNextItemWidth(-1);
              if (ImGui::ColorEdit3("##_color", color->data())) {
                light.m_json->Get(u8"color")->Set(*color);
              }
            } else {
              float white[]{ 1, 1, 1 };
              ImGui::ColorEdit3("##_color", white);
            }

            // 3
            ImGui::TableNextColumn();
            if (auto intensity = light.Intensity()) {
              ImGui::SetNextItemWidth(-1);
              ImGui::InputFloat("##_intensity", intensity);
            }

            ImGui::PopID();
          }
          ImGui::EndTable();
        }
      } else {
        ImGui::TextUnformatted("no lights");
      }
    }
  }
};

Lighting::Lighting()
  : m_impl(new LightingImpl)
{
}

Lighting::~Lighting()
{
  delete m_impl;
}

bool
Lighting::LoadHdr(const std::filesystem::path& path)
{
  return m_impl->LoadHdr(path);
}

void
Lighting::SetGltf(const std::shared_ptr<libvrm::GltfRoot>& root)
{
  m_impl->SetGltf(root);
}

void
Lighting::ShowGui()
{
  m_impl->ShowGui();
}
