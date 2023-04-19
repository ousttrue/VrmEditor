#pragma once
#include "json_gui.h"
#include "json_gui_table.h"
#include <imgui.h>

inline std::optional<int>
GetIndex(const std::shared_ptr<libvrm::gltf::Scene>& scene,
         std::string_view jsonpath)
{
  if (auto i = libvrm::JsonPath(jsonpath).GetLastInt()) {
    return *i;
  } else {
    return (int)scene->m_gltf.Json.at(
      nlohmann::json::json_pointer(jsonpath.data()));
  }
}

inline ShowGui
JsonGuiAccessorUShort4(std::span<const ushort4> items)
{
  return [items]() {
    ImGui::Text("ushort4[%zu]", items.size());
    std::array<const char*, 5> cols = {
      "index", "x", "y", "z", "w",
    };
    if (JsonGuiTable("##accessor_values", cols)) {
      ImGuiListClipper clipper;
      clipper.Begin(items.size());
      while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
          auto& value = items[i];
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
  };
}

inline ShowGui
JsonGuiAccessorVec2(std::span<const DirectX::XMFLOAT2> items)
{
  return [items]() {
    ImGui::Text("float2[%zu]", items.size());
    std::array<const char*, 3> cols = {
      "index",
      "x",
      "y",
    };
    if (JsonGuiTable("##accessor_values", cols)) {
      ImGuiListClipper clipper;
      clipper.Begin(items.size());
      while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
          auto& value = items[i];
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
  };
}

inline ShowGui
JsonGuiAccessorVec3(std::span<const DirectX::XMFLOAT3> items)
{
  return [items]() {
    ImGui::Text("float3[%zu]", items.size());
    std::array<const char*, 4> cols = {
      "index",
      "x",
      "y",
      "z",
    };
    if (JsonGuiTable("##accessor_values", cols)) {
      ImGuiListClipper clipper;
      clipper.Begin(items.size());
      while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
          auto& value = items[i];
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
  };
}

inline ShowGui
JsonGuiAccessorVec4(std::span<const DirectX::XMFLOAT4> items)
{
  return [items]() {
    ImGui::Text("float4[%zu]", items.size());
    std::array<const char*, 5> cols = {
      "index", "x", "y", "z", "w",
    };
    if (JsonGuiTable("##accessor_values", cols)) {
      ImGuiListClipper clipper;
      clipper.Begin(items.size());
      while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
          auto& value = items[i];
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
  };
}

inline ShowGui
JsonGuiAccessorMat4(std::span<const DirectX::XMFLOAT4X4> items)
{
  return [items]() {
    ImGui::Text("mat4[%zu]", items.size());
    std::array<const char*, 1 + 16> cols = {
      "index", "_11", "_12", "_13", "_14", "_21", "_22", "_23", "_24",
      "_31",   "_32", "_33", "_34", "_41", "_42", "_43", "_44",
    };
    if (JsonGuiTable("##accessor_values", cols)) {
      ImGuiListClipper clipper;
      clipper.Begin(items.size());
      while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
          auto& value = items[i];
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
  };
}

inline ShowGui
JsonGuiAccessor(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                std::string_view jsonpath)
{
  if (auto accessor_index = GetIndex(scene, jsonpath)) {
    auto accessor = scene->m_gltf.Json.at("accessors").at(*accessor_index);
    if (accessor.at("componentType") == 5123) {
      // ushort
      if (accessor.at("type") == "VEC4") {
        if (auto values = scene->m_gltf.accessor<ushort4>(*accessor_index)) {
          return JsonGuiAccessorUShort4(*values);
        }
      }
    } else if (accessor.at("componentType") == 5126) {
      // float
      if (accessor.at("type") == "VEC2") {
        if (auto values =
              scene->m_gltf.accessor<DirectX::XMFLOAT2>(*accessor_index)) {
          return JsonGuiAccessorVec2(*values);
        }
      } else if (accessor.at("type") == "VEC3") {
        if (auto values =
              scene->m_gltf.accessor<DirectX::XMFLOAT3>(*accessor_index)) {
          return JsonGuiAccessorVec3(*values);
        }
      } else if (accessor.at("type") == "VEC4") {
        if (auto values =
              scene->m_gltf.accessor<DirectX::XMFLOAT4>(*accessor_index)) {
          return JsonGuiAccessorVec4(*values);
        }
      } else if (accessor.at("type") == "MAT4") {
        if (auto values =
              scene->m_gltf.accessor<DirectX::XMFLOAT4X4>(*accessor_index)) {
          return JsonGuiAccessorMat4(*values);
        }
      }
    }

    int count = accessor.at("count");
    return [count]() { ImGui::Text("%d", count); };
  }

  return []() {};
}
