#include <GL/glew.h>

#include "scene_gui_material.h"
#include <glr/gl3renderer.h>
#include <grapho/gl3/texture.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <sstream>
#include <vrm/gltf.h>

static void
SelectTexture(const char* label,
              const std::shared_ptr<libvrm::gltf::GltfRoot>& root,
              std::shared_ptr<libvrm::gltf::Texture>* texture)
{
  // sampler
  using TUPLE = std::tuple<std::shared_ptr<libvrm::gltf::Texture>, std::string>;
  std::vector<TUPLE> combo;
  std::stringstream ss;
  int i = 0;
  for (auto& texture : root->m_textures) {
    ss.str("");
    ss << "[" << (i++) << "]" << texture->Name;
    combo.push_back({ texture, ss.str() });
  }
  std::span<TUPLE> span(combo.data(), combo.size());
  grapho::imgui::GenericCombo<std::shared_ptr<libvrm::gltf::Texture>>(
    label, texture, span);

  // source
  if (auto glTexture =
        glr::GetOrCreate(*texture, libvrm::gltf::ColorSpace::sRGB)) {
    ImGui::Image((ImTextureID)(uint64_t)glTexture->texture_, { 150, 150 });
    ImGui::SameLine();
  }
  ImGui::TextUnformatted(label);
}

void
ShowMaterialPbr(const std::shared_ptr<libvrm::gltf::GltfRoot>& root,
                const std::shared_ptr<libvrm::gltf::Material>& material)
{
  ImGui::Text("%s", material->Name.c_str());
  ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
  ImGui::TextUnformatted("Pbr");
  ImGui::PopStyleColor();

  // PBR
  // ImGui::SetNextItemOpen(true, ImGuiTreeNodeFlags_DefaultOpen);
  if (ImGui::CollapsingHeader("PbrMetallicRoughness")) {
    ImGui::ColorEdit4("BaseColorFactor", &material->Pbr.BaseColorFactor[0]);
    SelectTexture("BaseColorTexture", root, &material->Pbr.BaseColorTexture);
    ImGui::SliderFloat("MetallicFactor", &material->Pbr.MetallicFactor, 0, 1);
    ImGui::SliderFloat("RoughnessFactor", &material->Pbr.RoughnessFactor, 0, 1);
    SelectTexture("MetallicRoughnessTexture",
                  root,
                  &material->Pbr.MetallicRoughnessTexture);
  }

  SelectTexture("NormalTexture", root, &material->NormalTexture);
  ImGui::SliderFloat("NormalTextureScale", &material->NormalTextureScale, 0, 1);
  SelectTexture("OcclusionTexture", root, &material->OcclusionTexture);
  ImGui::SliderFloat(
    "OcclusionTextureStrength", &material->OcclusionTextureStrength, 0, 1);
  SelectTexture("EmissiveTexture", root, &material->EmissiveTexture);
  ImGui::ColorEdit3("EmissiveFactor", &material->EmissiveFactor[0]);
  grapho::imgui::EnumCombo(
    "AlphaMode", &material->AlphaMode, gltfjson::format::AlphaModesCombo);
  ImGui::SliderFloat("AlphaCutoff", &material->AlphaCutoff, 0, 1);
  ImGui::Checkbox("DoubleSided", &material->DoubleSided);
}

void
ShowMaterialUnlit(const std::shared_ptr<libvrm::gltf::GltfRoot>& root,
                  const std::shared_ptr<libvrm::gltf::Material>& material)
{
}

void
ShowMaterialMToon0(const std::shared_ptr<libvrm::gltf::GltfRoot>& root,
                   const std::shared_ptr<libvrm::gltf::Material>& material)
{
}

void
ShowMaterialMToon1(const std::shared_ptr<libvrm::gltf::GltfRoot>& root,
                   const std::shared_ptr<libvrm::gltf::Material>& material)
{
}
