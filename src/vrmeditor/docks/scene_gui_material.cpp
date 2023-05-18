#include <GL/glew.h>

#include "scene_gui_material.h"
#include <glr/gl3renderer.h>
#include <grapho/gl3/texture.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>

static void
ShowTexture(const char* label,
            const std::shared_ptr<libvrm::gltf::Texture>& texture)
{
  // sampler

  // source
  if (auto glTexture =
        glr::GetOrCreate(texture, libvrm::gltf::ColorSpace::sRGB)) {
    ImGui::Image((ImTextureID)(uint64_t)glTexture->texture_, { 150, 150 });
    ImGui::SameLine();
  }
  ImGui::TextUnformatted(label);

  if (texture) {
    grapho::imgui::EnumCombo(
      "ColorSpace", &texture->ColorSpace, libvrm::gltf::ColorSpaceCombo);
  }
}

void
ShowMaterialPbr(libvrm::gltf::Material& material)
{
  ImGui::Text("%s", material.Name.c_str());
  ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
  ImGui::TextUnformatted("Pbr");
  ImGui::PopStyleColor();

  // PBR
  // ImGui::SetNextItemOpen(true, ImGuiTreeNodeFlags_DefaultOpen);
  if (ImGui::CollapsingHeader("PbrMetallicRoughness")) {
    ImGui::ColorEdit4("BaseColorFactor", &material.Pbr.BaseColorFactor[0]);
    ShowTexture("BaseColorTexture", material.Pbr.BaseColorTexture);
    ImGui::SliderFloat("MetallicFactor", &material.Pbr.MetallicFactor, 0, 1);
    ImGui::SliderFloat("RoughnessFactor", &material.Pbr.RoughnessFactor, 0, 1);
    ShowTexture("MetallicRoughnessTexture",
                material.Pbr.MetallicRoughnessTexture);
  }

  ShowTexture("NormalTexture", material.NormalTexture);
  ImGui::SliderFloat("NormalTextureScale", &material.NormalTextureScale, 0, 1);
  ShowTexture("OcclusionTexture", material.OcclusionTexture);
  ImGui::SliderFloat(
    "OcclusionTextureStrength", &material.OcclusionTextureStrength, 0, 1);
  ShowTexture("EmissiveTexture", material.EmissiveTexture);
  ImGui::ColorEdit3("EmissiveFactor", &material.EmissiveFactor[0]);
  grapho::imgui::EnumCombo(
    "AlphaMode", &material.AlphaMode, gltfjson::format::AlphaModesCombo);
  ImGui::SliderFloat("AlphaCutoff", &material.AlphaCutoff, 0, 1);
  ImGui::Checkbox("DoubleSided", &material.DoubleSided);
}

void
ShowMaterialUnlit(libvrm::gltf::Material& material)
{
}

void
ShowMaterialMToon0(libvrm::gltf::Material& material)
{
}

void
ShowMaterialMToon1(libvrm::gltf::Material& material)
{
}
