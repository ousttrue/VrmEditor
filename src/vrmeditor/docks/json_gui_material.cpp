#include "json_gui_material.h"
#include "json_gui_table.h"
#include <imgui.h>
// #include <vrm/gltf.h>
#include <vrm/material.h>

struct MaterialItem
{
  std::string Name;
  std::string MaterialType;
};

ShowGui
JsonGuiMaterialList(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                    std::string_view jsonpath)
{
  std::vector<MaterialItem> items;
  for (auto& material : scene->m_gltf.Materials) {
    items.push_back({
      material.Name,
      "Pbr",
    });
    // if (libvrm::gltf::has(material, "extensions")) {
    //   auto& extensions = material.at("extensions");
    //   if (libvrm::gltf::has(extensions, "KHR_materials_unlit")) {
    //     items.back().MaterialType = "UnLit";
    //   }
    // }
  }
  for (int i = 0; i < scene->m_materials.size(); ++i) {
    auto& material = scene->m_materials[i];
    switch (material->Type) {
      case libvrm::gltf::MaterialTypes::Pbr:
        items[i].MaterialType = "Pbr";
        break;
      case libvrm::gltf::MaterialTypes::UnLit:
        items[i].MaterialType = "UnLit";
        break;
      case libvrm::gltf::MaterialTypes::MToon0:
        items[i].MaterialType = "MToon0";
        break;
      case libvrm::gltf::MaterialTypes::MToon1:
        items[i].MaterialType = "MToon1";
        break;
    }
  }
  return [items, &materials = scene->m_gltf.Materials]() {
    std::array<const char*, 6> cols = {
      "index", "name", "type", "alphamode", "colorFactor", "colorTexture",
    };
    if (JsonGuiTable("##materials", cols)) {
      for (int i = 0; i < items.size(); ++i) {
        auto& item = items[i];
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", i);

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", item.Name.c_str());

        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%s", item.MaterialType.c_str());

        ImGui::TableSetColumnIndex(3);
        auto& material = materials[i];
        // ImGui::Text("%s", material.AlphaMode).c_str());

        if (auto pbrMetallicRoughness = material.PbrMetallicRoughness) {
          auto color = pbrMetallicRoughness->BaseColorFactor;
          ImGui::TableSetColumnIndex(4);
          ImGui::ColorButton("##colorFactor", *((const ImVec4*)&color));

          ImGui::TableSetColumnIndex(5);
        }
      }
      ImGui::EndTable();
    }
  };

  return []() {};
}
