#include "json_gui.h"
#include "jsonpath_gui.h"
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
  : m_definitionMap({
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/glTF.schema.json
        u8"/",
        { {
          { u8"asset", u8"📄", {}, JsonPropFlags::Requried },
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
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/asset.schema.json
        u8"/asset",
        { {
          { u8"version",
            u8"📄",
            {},
            JsonPropFlags::Requried | JsonPropFlags::ReadOnly },
          { u8"minVersion", u8"📄" },
          { u8"copyright", u8"📄" },
          { u8"generator", u8"📄" },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/buffer.schema.json
        u8"/buffers/*",
        { {
          {
            u8"byteLength",
            u8"🔢",
            {},
            JsonPropFlags::Requried | JsonPropFlags::ReadOnly,
          },
          {
            u8"uri",
            u8"📄",
            {},
            JsonPropFlags::ReadOnly,
          },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/bufferView.schema.json
        u8"/bufferViews/*",
        { {
          { u8"buffer",
            u8"🆔",
            {},
            JsonPropFlags::Requried | JsonPropFlags::ReadOnly },
          { u8"byteLength",
            u8"📄",
            {},
            JsonPropFlags::Requried | JsonPropFlags::ReadOnly },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/accessor.schema.json
        u8"/accessors/*",
        { {
          { u8"componentType",
            u8"🔢",
            {},
            JsonPropFlags::Requried | JsonPropFlags::ReadOnly },
          { u8"type",
            u8"📄",
            {},
            JsonPropFlags::Requried | JsonPropFlags::ReadOnly },
          { u8"count",
            u8"🔢",
            {},
            JsonPropFlags::Requried | JsonPropFlags::ReadOnly },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/image.schema.json
        u8"/images/*",
        { {
          { u8"uri", u8"📄" },
          { u8"mimeType", u8"📄" },
          { u8"bufferView", u8"🆔" },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/sampler.schema.json
        u8"/samplers/*",
        { {
          { u8"magFilter", u8"🔢" },
          { u8"minFilter", u8"🔢" },
          { u8"wrapS", u8"🔢" },
          { u8"wrapT", u8"🔢" },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/texture.schema.json
        u8"/textures/*",
        { {
          { u8"source", u8"🆔" },
          { u8"sampler", u8"🆔" },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/material.schema.json
        u8"/materials/*",
        { {
          { u8"pbrMetallicRoughness", u8"💎" },
          { u8"normalTexture", u8"🖼" },
          { u8"occlusionTexture", u8"🖼" },
          { u8"emissiveTexture", u8"🖼" },
          { u8"emissiveFactor", u8"🎨", RgbPicker{ .Default = { 0, 0, 0 } } },
          { u8"alphaMode", u8"📄" },
          { u8"alphaCutoff", u8"🎚️" },
          { u8"doubleSided", u8"✅" },
        } },
      },
      {
        // https : //
        // github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/mesh.primitive.schema.json
        u8"/meshes/*/primitives/*",
        { {
          { u8"attributes",
            u8"📄",
            {},
            JsonPropFlags::Requried | JsonPropFlags::ReadOnly },
          { u8"indices", u8"📄", {}, JsonPropFlags::ReadOnly },
          { u8"material", u8"🆔" },
        } },
      },
      {
        // https : //
        // github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/mesh.schema.json
        u8"/meshes/*",
        { {
          { u8"primitives", u8"[]", {}, JsonPropFlags::Requried },
          { u8"weights", u8"[]" },
        } },
      },
      {
        // https : //
        // github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/node.schema.json
        u8"/nodes/*",
        { {
          { u8"mesh", u8"🆔" },
        } },
      },
    })
{

  // m_inspector->OnUpdated([](auto jsonpath) {
  //   gltfjson::JsonPath path(jsonpath);
  //   auto [childOfRoot, i] = path.GetChildOfRootIndex();
  //   if (childOfRoot == u8"materials") {
  //     glr::ReleaseMaterial(i);
  //   }
  // });

  // m_inspector->OnUpdated(std::bind(&JsonGuiFactoryManager::ClearJsonPath,
  //                                  m_inspector.get(),
  //                                  std::placeholders::_1));
}

void
JsonGui::ClearCache()
{
  m_cacheMap.clear();
}

void
JsonGui::SetScene(const std::shared_ptr<libvrm::GltfRoot>& root)
{
  m_root = root;
  ClearCache();
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
  auto isSelected = jsonpath == m_jsonpath;
  if (isSelected) {
    node_flags |= ImGuiTreeNodeFlags_Selected;
  }

  ImGui::TableNextRow();

  // 0
  ImGui::TableNextColumn();
  if (ShouldOpen(jsonpath)) {
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  }

  int push = 0;
  if (item) {
    if (Has(prop.Flags, JsonPropFlags::Unknown)) {
      ImGui::PushStyleColor(ImGuiCol_Text, grapho::imcolor::yellow);
      ++push;
    }
  } else {
    ImGui::PushStyleColor(ImGuiCol_Text, grapho::imcolor::gray);
    ++push;
  }

  auto id = ImGui::GetID((const char*)jsonpath.c_str());

  Cache* cache = nullptr;
  auto found = m_cacheMap.find(jsonpath);
  if (found != m_cacheMap.end()) {
    cache = &found->second;
  } else {
    auto inserted =
      m_cacheMap.insert({ jsonpath, { prop.Label(), prop.Value(item) } });
    cache = &inserted.first->second;
    if (prop.Factory) {
      cache->Editor = prop.Factory(jsonpath);
    }
  }
  auto node_open = ImGui::TreeNodeEx(
    (void*)(intptr_t)id, node_flags, "%s", (const char*)cache->Label.data());
  ImGui::PopStyleColor(push);

  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    m_jsonpath = jsonpath;
  }

  // 1
  ImGui::TableNextColumn();
  if (isSelected && cache->Editor) {
    cache->Editor(m_root->m_gltf->m_json, m_root->m_bin, item);
  } else {
    ImGui::TextUnformatted((const char*)cache->value.c_str());
  }

  // 2
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

  std::array<const char*, 3> cols = {
    "Name",
    "Value",
    "✅",
  };

  if (grapho::imgui::BeginTableColumns("##JsonGui::ShowSelector", cols)) {

    // tree
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, indent);

    std::u8string jsonpath(u8"/");
    Traverse(m_root->m_gltf->m_json,
             jsonpath,
             JsonProp{ u8"glTF", u8"", {}, JsonPropFlags::Unknown });

    ImGui::PopStyleVar();

    ImGui::EndTable();
  }
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
      std::unordered_set<std::u8string> used;
      // used.clear();
      if (auto definition = m_definitionMap.Match(jsonpath)) {
        for (auto prop : definition->Props) {
          jsonpath += prop.Key;
          used.insert(prop.Key);
          if (auto child = item->Get(prop.Key)) {
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
          Traverse(
            v,
            jsonpath,
            { jsonpath.substr(size), u8"❔", {}, JsonPropFlags::Unknown });
          jsonpath.resize(size);
        }
      }
    } else if (auto array = item->Array()) {
      int i = 0;
      for (auto& v : *array) {
        gltfjson::tree::concat_int(jsonpath, i++);
        Traverse(
          v,
          jsonpath,
          { jsonpath.substr(size), prop.Icon, {}, JsonPropFlags::Unknown });
        jsonpath.resize(size);
      }
    }
    ImGui::TreePop();
  }
}
