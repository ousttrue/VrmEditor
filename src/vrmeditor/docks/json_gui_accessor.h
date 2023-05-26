#pragma once
#include "json_gui_table.h"
#include "showgui.h"
#include <array>
#include <string>

struct AccessorItem
{
  std::string Type;
  int Size;
  std::string Ref;
};

ShowGuiFunc
JsonGuiAccessorList(std::u8string_view jsonpath)
{
  return [items = std::vector<AccessorItem>()](
           const gltfjson::typing::Root& root,
           const gltfjson::typing::Bin& bin,
           const gltfjson::tree::NodePtr& node) mutable {
    if (items.empty()) {
      auto& accessors = root.Accessors;
      for (size_t i = 0; i < accessors.size(); ++i) {
        std::vector<std::string> refs;
        auto accessor = accessors[i];
        // "%s[%s]", .c_str());
        auto component = accessor.ComponentType();
        auto type = accessor.Type();
        {
          auto& meshes = root.Meshes;
          for (size_t j = 0; j < meshes.size(); ++j) {
            auto mesh = meshes[j];
            auto& prims = mesh.Primitives;
            for (size_t k = 0; k < prims.size(); ++k) {
              auto prim = prims[k];
              // if (libvrm::gltf::has(prim, "attributes")) {
              //   for (auto& kv : prim.at("attributes").items()) {
              //     if (kv.value() == i) {
              //       std::stringstream ss2;
              //       ss2 << "/meshes/" << j << "/primitives/" << k <<
              //       "/attributes/"
              //           << kv.key();
              //       refs.push_back(ss2.str());
              //     }
              //   }
              // }
              // if (libvrm::gltf::has(prim, "indices")) {
              //   if (prim.at("indices") == i) {
              //     std::stringstream ss2;
              //     ss2 << "/meshes/" << j << "/primitives/" << k <<
              //     "/indices"; refs.push_back(ss2.str());
              //   }
              // }
              // if (libvrm::gltf::has(prim, "targets")) {
              //   auto& targets = prim.at("targets");
              //   for (size_t l = 0; l < targets.size(); ++l) {
              //     auto& target = targets[l];
              //     for (auto& kv : target.items()) {
              //       if (kv.value() == i) {
              //         std::stringstream ss2;
              //         ss2 << "/meshes/" << j << "/primitives/" << k <<
              //         "/targets/"
              //             << l << "/" << kv.key();
              //         refs.push_back(ss2.str());
              //       }
              //     }
              //   }
              // }
            }
          }
        }
        // if (libvrm::gltf::has(scene->m_gltf.Json, "skins")) {
        //   auto& skins = scene->m_gltf.Json.at("skins");
        //   for (size_t j = 0; j < skins.size(); ++j) {
        //     auto& skin = skins[j];
        //     if (libvrm::gltf::has(skin, "inverseBindMatrices")) {
        //       if (skin.at("inverseBindMatrices") == i) {
        //         std::stringstream ss2;
        //         ss2 << "/skins/" << j << "/inverseBindMatrices";
        //         refs.push_back(ss2.str());
        //       }
        //     }
        //   }
        // }

        {
          auto count = (int)*accessor.Count();
          // std::stringstream ss;
          // ss << libvrm::gltf::component_type_name(component, type) << "["
          //    << *accessor.Count() << "]";
          items.push_back({
            "TYPE",
            static_cast<int>(*gltfjson::format::component_size(
                               (gltfjson::format::ComponentTypes)*component) *
                             *gltfjson::format::type_count(type) * count),
          });
        }
        {
          std::stringstream ss2;
          size_t i = 0;
          for (auto& ref : refs) {
            ss2 << '[' << (i++) << ']' << ref;
          }
          items.back().Ref = ss2.str();
        }
      }
    }
    std::array<const char*, 4> cols = {
      "index",
      "type",
      "size",
      "ref",
    };
    if (JsonGuiTable("##accessors", cols)) {
      for (int i = 0; i < items.size(); ++i) {
        auto& item = items[i];
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", i);

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", item.Type.c_str());

        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%d", item.Size);

        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%s", item.Ref.c_str());
      }
      ImGui::EndTable();
    }
  };
}
