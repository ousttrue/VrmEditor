#include "json_gui_material.h"
#include "json_gui_table.h"
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <vrm/gltf.h>
#include <vrm/json.h>
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
  if (libvrm::gltf::has(scene->m_gltf.Json, "materials")) {
    std::vector<MaterialItem> items;
    auto& materials = scene->m_gltf.Json.at("materials");
    for (auto& material : materials) {
      items.push_back({
        material.value("name", "no name"),
        "Pbr",
      });
      if (libvrm::gltf::has(material, "extensions")) {
        auto& extensions = material.at("extensions");
        if (libvrm::gltf::has(extensions, "KHR_materials_unlit")) {
          items.back().MaterialType = "UnLit";
        }
      }
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
    return [items, materials]() {
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
          ImGui::Text(
            "%s", material.value("alphaMode", std::string("OPAQUE")).c_str());

          if (libvrm::gltf::has(material, "pbrMetallicRoughness")) {
            auto& pbrMetallicRoughness = material.at("pbrMetallicRoughness");
            auto color = pbrMetallicRoughness.value(
              "baseColorFactor", DirectX::XMFLOAT4{ 1, 1, 1, 1 });
            ImGui::TableSetColumnIndex(4);
            ImGui::ColorButton("##colorFactor", *((const ImVec4*)&color));

            ImGui::TableSetColumnIndex(5);
          }
        }
        ImGui::EndTable();
      }
    };
  }

  return []() {};
}
