#pragma once
#include "json_gui.h"
#include <array>
#include <boneskin/types.h>
#include <gltfjson.h>
#include <gltfjson/bin.h>
#include <gltfjson/jsonpath.h>
#include <grapho/imgui/widgets.h>
#include <optional>
#include <string>

static std::optional<int>
GetIndex(std::u8string_view jsonpath)
{
  if (auto i = gltfjson::JsonPath(jsonpath).GetLastInt()) {
    return *i;
  } else {
    return std::nullopt;
  }
}

static void
JsonGuiAccessorUShort4(const gltfjson::Root& root,
                       const gltfjson::Bin& bin,
                       const gltfjson::tree::NodePtr& node)
{
  if (auto items = bin.GetAccessorBytes<boneskin::ushort4>(
        root, (int)*node->Ptr<float>())) {
    ImGui::Text("ushort4[%zu]", items->size());
    std::array<const char*, 5> cols = {
      "index", "x", "y", "z", "w",
    };
    if (grapho::imgui::BeginTableColumns("##accessor_values", cols)) {
      ImGuiListClipper clipper;
      clipper.Begin(items->size());
      while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
          auto& value = (*items)[i];
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("%d", i);
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%u", value.X);
          ImGui::TableSetColumnIndex(2);
          ImGui::Text("%u", value.Y);
          ImGui::TableSetColumnIndex(3);
          ImGui::Text("%u", value.Z);
          ImGui::TableSetColumnIndex(4);
          ImGui::Text("%u", value.W);
        }
      }
      ImGui::EndTable();
    }
  }
}

static void
JsonGuiAccessorVec2(const gltfjson::Root& root,
                    const gltfjson::Bin& bin,
                    const gltfjson::tree::NodePtr& node)
{
  if (auto items = bin.GetAccessorBytes<DirectX::XMFLOAT2>(
        root, (int)*node->Ptr<float>())) {
    ImGui::Text("float2[%zu]", items->size());
    std::array<const char*, 3> cols = {
      "index",
      "x",
      "y",
    };
    if (grapho::imgui::BeginTableColumns("##accessor_values", cols)) {
      ImGuiListClipper clipper;
      clipper.Begin(items->size());
      while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
          auto& value = (*items)[i];
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("%d", i);
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%f", value.x);
          ImGui::TableSetColumnIndex(2);
          ImGui::Text("%f", value.y);
        }
      }
      ImGui::EndTable();
    }
  }
}

static void
JsonGuiAccessorVec3(const gltfjson::Root& root,
                    const gltfjson::Bin& bin,
                    const gltfjson::tree::NodePtr& node)
{
  if (auto items = bin.GetAccessorBytes<DirectX::XMFLOAT3>(
        root, (int)*node->Ptr<float>())) {
    ImGui::Text("float3[%zu]", items->size());
    std::array<const char*, 4> cols = {
      "index",
      "x",
      "y",
      "z",
    };
    if (grapho::imgui::BeginTableColumns("##accessor_values", cols)) {
      ImGuiListClipper clipper;
      clipper.Begin(items->size());
      while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
          auto& value = (*items)[i];
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("%d", i);
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%f", value.x);
          ImGui::TableSetColumnIndex(2);
          ImGui::Text("%f", value.y);
          ImGui::TableSetColumnIndex(3);
          ImGui::Text("%f", value.z);
        }
      }
      ImGui::EndTable();
    }
  }
}

static void
JsonGuiAccessorVec4(const gltfjson::Root& root,
                    const gltfjson::Bin& bin,
                    const gltfjson::tree::NodePtr& node)
{
  if (auto items = bin.GetAccessorBytes<DirectX::XMFLOAT4>(
        root, (int)*node->Ptr<float>())) {
    ImGui::Text("float4[%zu]", items->size());
    std::array<const char*, 5> cols = {
      "index", "x", "y", "z", "w",
    };
    if (grapho::imgui::BeginTableColumns("##accessor_values", cols)) {
      ImGuiListClipper clipper;
      clipper.Begin(items->size());
      while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
          auto& value = (*items)[i];
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("%d", i);
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%f", value.x);
          ImGui::TableSetColumnIndex(2);
          ImGui::Text("%f", value.y);
          ImGui::TableSetColumnIndex(3);
          ImGui::Text("%f", value.z);
          ImGui::TableSetColumnIndex(4);
          ImGui::Text("%f", value.w);
        }
      }
      ImGui::EndTable();
    }
  }
}

static void
JsonGuiAccessorMat4(const gltfjson::Root& root,
                    const gltfjson::Bin& bin,
                    const gltfjson::tree::NodePtr& node)
{
  if (auto items = bin.GetAccessorBytes<DirectX::XMFLOAT4X4>(
        root, (int)*node->Ptr<float>())) {
    ImGui::Text("mat4[%zu]", items->size());
    std::array<const char*, 1 + 16> cols = {
      "index", "_11", "_12", "_13", "_14", "_21", "_22", "_23", "_24",
      "_31",   "_32", "_33", "_34", "_41", "_42", "_43", "_44",
    };
    if (grapho::imgui::BeginTableColumns("##accessor_values", cols)) {
      ImGuiListClipper clipper;
      clipper.Begin(items->size());
      while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
          auto& value = (*items)[i];
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("%d", i);
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%f", value._11);
          ImGui::TableSetColumnIndex(2);
          ImGui::Text("%f", value._12);
          ImGui::TableSetColumnIndex(3);
          ImGui::Text("%f", value._13);
          ImGui::TableSetColumnIndex(4);
          ImGui::Text("%f", value._14);
          ImGui::TableSetColumnIndex(5);
          ImGui::Text("%f", value._21);
          ImGui::TableSetColumnIndex(6);
          ImGui::Text("%f", value._22);
          ImGui::TableSetColumnIndex(7);
          ImGui::Text("%f", value._23);
          ImGui::TableSetColumnIndex(8);
          ImGui::Text("%f", value._24);
          ImGui::TableSetColumnIndex(9);
          ImGui::Text("%f", value._31);
          ImGui::TableSetColumnIndex(10);
          ImGui::Text("%f", value._32);
          ImGui::TableSetColumnIndex(11);
          ImGui::Text("%f", value._33);
          ImGui::TableSetColumnIndex(12);
          ImGui::Text("%f", value._34);
          ImGui::TableSetColumnIndex(13);
          ImGui::Text("%f", value._41);
          ImGui::TableSetColumnIndex(14);
          ImGui::Text("%f", value._42);
          ImGui::TableSetColumnIndex(15);
          ImGui::Text("%f", value._43);
          ImGui::TableSetColumnIndex(16);
          ImGui::Text("%f", value._44);
        }
      }
      ImGui::EndTable();
    }
  }
}

ShowGuiFunc
JsonGuiAccessorReference(std::u8string_view jsonpath)
{
  return [](const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            const gltfjson::tree::NodePtr& node) {
    auto accessor_index = (int)*node->Ptr<float>();
    auto accessor = root.Accessors[accessor_index];
    auto component_type = (gltfjson::ComponentTypes)*accessor.ComponentType();
    auto type = accessor.Type();
    if (component_type == gltfjson::ComponentTypes::UNSIGNED_SHORT) {
      // ushort
      if (type == u8"VEC4") {
        JsonGuiAccessorUShort4(root, bin, node);
      }
    } else if (component_type == gltfjson::ComponentTypes::FLOAT) {
      // float
      if (type == u8"VEC2") {
        JsonGuiAccessorVec2(root, bin, node);
      } else if (type == u8"VEC3") {
        JsonGuiAccessorVec3(root, bin, node);
      } else if (type == u8"VEC4") {
        JsonGuiAccessorVec4(root, bin, node);
      } else if (type == u8"MAT4") {
        JsonGuiAccessorMat4(root, bin, node);
      }
    }
    return false;
  };
}

struct AccessorItem
{
  std::string Type;
  int Size;
  std::string Ref;
};

inline ShowGuiFunc
JsonGuiAccessorList(std::u8string_view jsonpath)
{
  return [items = std::vector<AccessorItem>()](
           const gltfjson::Root& root,
           const gltfjson::Bin& bin,
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
            static_cast<int>(
              *gltfjson::component_size((gltfjson::ComponentTypes)*component) *
              *gltfjson::type_count(type) * count),
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
    if (grapho::imgui::BeginTableColumns("##accessors", cols)) {
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
    return false;
  };
}
