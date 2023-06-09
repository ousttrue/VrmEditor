#include "json_gui.h"
#include "json_gui_factory.h"
#include <array>
#include <charconv>
#include <glr/gl3renderer.h>
#include <gltfjson.h>
#include <grapho/imgui/csscolor.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <ranges>
#include <sstream>
#include <string_view>
#include <unordered_set>

JsonGui::JsonGui()
  : m_inspector(new JsonGuiFactoryManager)
  , m_definitionMap({
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/glTF.schema.json
        u8"/",
        { {
          { u8"asset", JsonPropFlags::Requried },
          //
          { u8"buffers" },
          { u8"bufferViews" },
          { u8"accessors" },
          //
          { u8"images" },
          { u8"samplers" },
          { u8"textures" },
          { u8"materials" },
          { u8"meshes" },
          { u8"nodes" },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/buffer.schema.json
        u8"/buffers/*",
        { {
          { u8"byteLength", JsonPropFlags::Requried | JsonPropFlags::ReadOnly },
          { u8"uri", JsonPropFlags::ReadOnly },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/bufferView.schema.json
        u8"/bufferViews/*",
        { {
          { u8"buffer", JsonPropFlags::Requried | JsonPropFlags::ReadOnly },
          { u8"byteLength", JsonPropFlags::Requried | JsonPropFlags::ReadOnly },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/accessor.schema.json
        u8"/accessors/*",
        { {
          { u8"componentType",
            JsonPropFlags::Requried | JsonPropFlags::ReadOnly },
          { u8"type", JsonPropFlags::Requried | JsonPropFlags::ReadOnly },
          { u8"count", JsonPropFlags::Requried | JsonPropFlags::ReadOnly },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/asset.schema.json
        u8"/asset",
        { {
          { u8"version", JsonPropFlags::Requried | JsonPropFlags::ReadOnly },
          { u8"minVersion" },
          { u8"copyright" },
          { u8"generator" },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/image.schema.json
        u8"/images/*",
        { {
          { u8"uri" },
          { u8"mimeType" },
          { u8"bufferView" },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/sampler.schema.json
        u8"/samplers/*",
        { {
          { u8"magFilter" },
          { u8"minFilter" },
          { u8"wrapS" },
          { u8"wrapT" },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/texture.schema.json
        u8"/textures/*",
        { {
          { u8"source" },
          { u8"sampler" },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/material.schema.json
        u8"/materials/*",
        { {
          { u8"pbrMetallicRoughness" },
          { u8"normalTexture" },
          { u8"occlusionTexture" },
          { u8"emissiveTexture" },
          { u8"emissiveFactor" },
          { u8"alphaMode" },
          { u8"alphaCutoff" },
          { u8"doubleSided" },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/mesh.primitive.schema.json
        u8"/meshes/*/primitives/*",
        { {
          { u8"attributes", JsonPropFlags::Requried | JsonPropFlags::ReadOnly },
          { u8"indices", JsonPropFlags::ReadOnly },
          { u8"material" },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/mesh.schema.json
        u8"/meshes/*",
        { {
          { u8"primitives", JsonPropFlags::Requried },
          { u8"weights" },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/node.schema.json
        u8"/nodes/*",
        { {
          { u8"mesh" },
        } },
      },
    })
{

  m_inspector->OnUpdated([](auto jsonpath) {
    gltfjson::JsonPath path(jsonpath);
    auto [childOfRoot, i] = path.GetChildOfRootIndex();
    if (childOfRoot == u8"materials") {
      glr::ReleaseMaterial(i);
    }
  });

  m_inspector->OnUpdated(std::bind(&JsonGuiFactoryManager::ClearJsonPath,
                                   m_inspector.get(),
                                   std::placeholders::_1));
}

void
JsonGui::SetScene(const std::shared_ptr<libvrm::GltfRoot>& root)
{
  m_root = root;
  m_inspector->ClearAll();
}

bool
JsonGui::Enter(const gltfjson::tree::NodePtr& item,
               const std::u8string& jsonpath,
               const JsonProp& prop)
{
  static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                         ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                         ImGuiTreeNodeFlags_SpanAvailWidth;
  ImGuiTreeNodeFlags node_flags = base_flags;
  auto is_leaf = (!item || item->Size() == 0);
  if (is_leaf) {
    node_flags |=
      ImGuiTreeNodeFlags_Leaf |
      ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
  }
  if (m_inspector->IsSelected(jsonpath)) {
    node_flags |= ImGuiTreeNodeFlags_Selected;
  }

  auto& label = m_inspector->Get(jsonpath, item);

  ImGui::TableNextRow();

  // 0
  ImGui::TableNextColumn();
  if (m_inspector->ShouldOpen(jsonpath)) {
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  }
  bool node_open = false;
  if (item) {
    if (Has(prop.Flags, JsonPropFlags::Unknown)) {
      ImGui::PushStyleColor(ImGuiCol_Text, grapho::imcolor::yellow);
    }
    node_open = ImGui::TreeNodeEx((void*)(intptr_t)item.get(),
                                  node_flags,
                                  "%s",
                                  (const char*)label.Key.c_str());
    if (Has(prop.Flags, JsonPropFlags::Unknown)) {
      ImGui::PopStyleColor();
    }
  } else {
    ImGui::PushStyleColor(ImGuiCol_Text, grapho::imcolor::gray);
    // ImGui::TextUnformatted((const char*)label.Key.c_str());
    node_open = ImGui::TreeNodeEx((const char*)jsonpath.data(),
                                  node_flags,
                                  "%s",
                                  (const char*)label.Key.c_str());
    ImGui::PopStyleColor();
  }
  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    m_inspector->Select(jsonpath);
  }

  // 1
  ImGui::TableNextColumn();
  if (Has(prop.Flags, JsonPropFlags::Unknown)) {
    //
  } else if (Has(prop.Flags, JsonPropFlags::Requried)) {
    //
  } else {
    bool value = item != nullptr;
    if (ImGui::Checkbox(
          m_buf.Printf("##enable_%s", (const char*)jsonpath.data()), &value)) {
      if (item) {
        // remove
      } else {
        // create
      }
    }
  }

  // 2
  ImGui::TableNextColumn();
  ImGui::TextUnformatted((const char*)label.Value.c_str());

  return node_open && !is_leaf;
}

void
JsonGui::ShowSelector(float indent)
{
  if (!m_root) {
    return;
  }
  if (!m_root->m_gltf) {
    return;
  }
  if (!m_root->m_gltf->m_json) {
    return;
  }

  // auto enter = [this](const gltfjson::tree::NodePtr& item,
  //                     std::u8string_view jsonpath) {
  //   return Enter(item, jsonpath);
  // };
  // auto leave = []() { ImGui::TreePop(); };

  std::array<const char*, 3> cols = {
    "Name",
    "",
    "Value",
  };

  if (grapho::imgui::BeginTableColumns("##JsonGui::ShowSelector", cols)) {

    // tree
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, indent);

    // gltfjson::tree::TraverseJson(enter, leave, m_root->m_gltf->m_json);
    std::u8string jsonpath(u8"/");
    Traverse(m_root->m_gltf->m_json,
             jsonpath,
             JsonProp{ u8"", JsonPropFlags::Unknown });

    ImGui::PopStyleVar();

    ImGui::EndTable();
  }
}

void
JsonGui::ShowSelected()
{
  if (!m_root) {
    return;
  }
  if (!m_root->m_gltf) {
    return;
  }
  if (!m_root->m_gltf->m_json) {
    return;
  }

  m_inspector->ShowGui(*m_root->m_gltf, m_root->m_bin);
}

void
JsonGui::Traverse(const gltfjson::tree::NodePtr& item,
                  std::u8string& jsonpath,
                  const JsonProp& prop)
{
  if (Enter(item, jsonpath, prop)) {
    gltfjson::tree::AddDelimiter(jsonpath);
    auto size = jsonpath.size();
    if (auto object = item->Object()) {
      static std::unordered_set<std::u8string> used;
      used.clear();
      if (auto definition = m_definitionMap.Match(jsonpath)) {
        for (auto prop : definition->Props) {
          jsonpath += prop.Key;
          if (auto child = item->Get(prop.Key)) {
            used.insert(prop.Key);
            Traverse(child, jsonpath, prop);
          } else {
            Traverse(nullptr, jsonpath, prop);
          }
          jsonpath.resize(size);
        }
      }

      for (auto [k, v] : *object) {
        if (used.find(k) == used.end()) {
          jsonpath += k;
          Traverse(v, jsonpath, { u8"", JsonPropFlags::Unknown });
          jsonpath.resize(size);
        }
      }
    } else if (auto array = item->Array()) {
      int i = 0;
      for (auto& v : *array) {
        gltfjson::tree::concat_int(jsonpath, i++);
        Traverse(v, jsonpath, { u8"", JsonPropFlags::Unknown });
        jsonpath.resize(size);
      }
    }
    ImGui::TreePop();
  }
}
