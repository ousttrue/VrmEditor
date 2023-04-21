#include "json_gui_material.h"
#include "json_gui_table.h"
#include <imgui.h>
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
  if (has(scene->m_gltf.Json, "materials")) {
    auto& materials = scene->m_gltf.Json.at("materials");
    for (auto& material : materials) {
      items.push_back({
        material.value("name", "no name"),
        "Pbr",
      });
      if (has(material, "extensions")) {
        auto& extensions = material.at("extensions");
        if (has(extensions, "KHR_materials_unlit")) {
          items.back().MaterialType = "UnLit";
        }
      }
    }
  }
  {
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
  }

  return [items]() {
    std::array<const char*, 3> cols = {
      "index",
      "name",
      "type",
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
      }
      ImGui::EndTable();
    }
  };
}
