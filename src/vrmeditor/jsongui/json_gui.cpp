#include "json_gui.h"
#include "json_widgets.h"
#include <array>
#include <charconv>
#include <glr/gl3renderer.h>
#include <gltfjson.h>
#include <grapho/imgui/csscolor.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <plog/Log.h>
#include <ranges>
#include <sstream>
#include <string_view>
#include <unordered_set>

inline const std::u8string
U8Q(const char* str)
{
  return std::u8string(u8"\"") + std::u8string((const char8_t*)str) +
         std::u8string(u8"\"");
}

JsonGui::JsonGui()
  : m_definitionMap({
      {
        // https : //
        // github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/glTF.schema.json
        u8"/",
        { {
          { u8"asset", u8"📄", { {}, u8"{}" }, JsonPropFlags::Required },
          //
          { u8"extensions", u8"⭐", { {}, u8"{}" } },
          { u8"extensionsUsed", u8"⭐", { {}, u8"[]" } },
          { u8"extensionsRequired", u8"⭐", { {}, u8"[]" } },
          { u8"extras", u8"⭐", { {}, u8"{}" } },
          //
          { u8"buffers", u8"📦", { {}, u8"[]" } },
          { u8"bufferViews", u8"📦", { {}, u8"[]" } },
          { u8"accessors", u8"📦", { {}, u8"[]" } },
          //
          { u8"images", u8"🖼", { {}, u8"[]" } },
          { u8"samplers", u8"🖼", { {}, u8"[]" } },
          { u8"textures", u8"🖼", { {}, u8"[]" } },
          { u8"materials", u8"💎", { {}, u8"[]" } },
          //
          { u8"meshes", u8"📐", { {}, u8"[]" } },
          { u8"skins", u8"📐", { {}, u8"[]" } },
          //
          { u8"nodes", u8"🛞", { {}, u8"[]" } },
          { u8"scenes", u8"🛞", { {}, u8"[]" } },
          { u8"scene", u8"🆔", { {}, u8"0" } },
        } },
      },
      {
        u8"/extensions",
        { {
          { u8"VRM", u8"🌟", { {}, u8"{}" } },
          { u8"VRMC_vrm", u8"🌟", { {}, u8"{}" } },
          { u8"VRMC_springBone", u8"🌟", { {}, u8"{}" } },
        } },
      },
      // VRM
      {
        // https : //
        // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.schema.json
        u8"/extensions/VRM",
        { {
          { u8"exporterVersion", u8"📄" },
          { u8"specVersion", u8"📄" },
          { u8"meta", u8"🪪" },
          { u8"humanoid", u8"👤" },
          { u8"firstPerson", u8"👀" },
          { u8"blendShapeMaster", u8"😀" },
          { u8"secondaryAnimation", u8"🔗" },
          { u8"materialProperties", u8"💎" },
        } },
      },
      {
        // https : //
        // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.meta.schema.json
        u8"/extensions/VRM/meta",
        { {
          { u8"title", u8"🪪" },
          { u8"version", u8"🪪" },
          { u8"author", u8"🪪" },
          { u8"contactInformation", u8"🪪" },
          { u8"reference", u8"🪪" },
          { u8"texture", u8"🆔" },
          { u8"allowedUserName", u8"🪪" },
          { u8"violentUssageName", u8"🪪" },
          { u8"sexualUssageName", u8"🪪" },
          { u8"commercialUssageName", u8"🪪" },
          { u8"otherPermissionUrl", u8"🪪" },
          { u8"licenseName", u8"🪪" },
          { u8"otherLicenseUrl", u8"🪪" },
        } },
      },
      {
        // https : //
        // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.humanoid.schema.json
        u8"/extensions/VRM/humanoid",
        { {
          { u8"humanBones", u8"🦴" },
          { u8"armStretch", u8"⛔" },
          { u8"legStretch", u8"⛔" },
          { u8"upperArmTwist", u8"⛔" },
          { u8"lowerArmTwist", u8"⛔" },
          { u8"upperLegTwist", u8"⛔" },
          { u8"lowerLegTwist", u8"⛔" },
          { u8"feetSpacing", u8"⛔" },
          { u8"hasTranslationDoF", u8"⛔" },
        } },
      },
      {
        // https : //
        // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.humanoid.bone.schema.json
        u8"/extensions/VRM/humanoid/humanBones/*",
        { {
          { u8"bone", u8"🦴" },
          { u8"node", u8"🆔" },
          { u8"useDefaultValues", u8"⛔" },
          { u8"min", u8"⛔" },
          { u8"max", u8"⛔" },
          { u8"center", u8"⛔" },
          { u8"axisLength", u8"⛔" },
        } },
      },
      {
        // https : //
        // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.firstperson.schema.json
        u8"/extensions/VRM/firstPerson",
        { {
          { u8"firstPersonBone", u8"🆔" },
          { u8"firstPersonBoneOffset", u8"↔" },
          { u8"meshAnnotations", u8"✨" },
          { u8"lookAtTypeName", u8"👀" },
          { u8"lookAtHorizontalInner", u8"👀" },
          { u8"lookAtHorizontalOuter", u8"👀" },
          { u8"lookAtVerticalDown", u8"👀" },
          { u8"lookAtVerticalUp", u8"👀" },
        } },
      },
      {
        // https : //
        // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.blendshape.schema.json
        u8"/extensions/VRM/blendShapeMaster",
        { {
          { u8"blendShapeGroups", u8"😀" },
        } },
      },
      {
        // https : //
        // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.blendshape.group.schema.json
        u8"/extensions/VRM/blendShapeMaster/blendShapeGroups/*",
        { {
          { u8"name", u8"📄" },
          { u8"presetName", u8"😀" },
          { u8"binds", u8"😀" },
          { u8"materialValues", u8"💎" },
          { u8"isBinary", u8"✅" },
        } },
      },
      {
        // https : //
        // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.secondaryanimation.schema.json
        u8"/extensions/VRM/secondaryanimation",
        { {
          { u8"boneGroups", u8"🔗" },
          { u8"colliderGroups", u8"🎱" },
        } },
      },
      {
        // https : //
        // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.material.schema.json
        u8"/extensions/VRM/materialProperties/*",
        { {
          { u8"name", u8"📄" },
          { u8"shader", u8"📄" },
          { u8"renderQueue", u8"🔢" },
          { u8"floatProperties", u8"🔢" },
          { u8"vectorProperties", u8"🔢" },
          { u8"textureProperties", u8"🖼" },
          { u8"keywordMap", u8"📄" },
          { u8"tagMap", u8"✅" },
        } },
      },
      // VRMC_vrm
      {
        // https : //
        // github.com/vrm-c/vrm-specification/blob/master/specification/VRMC_vrm-1.0/schema/VRMC_vrm.schema.json
        u8"/extensions/VRMC_vrm",
        { {
          { u8"specVersion", u8"📄" },
          { u8"meta", u8"🪪" },
          { u8"humanoid", u8"👤" },
          { u8"firstPerson", u8"✨" },
          { u8"lookAt", u8"👀" },
          { u8"expressions", u8"😀" },
        } },
      },
      {
        // https://github.com/vrm-c/vrm-specification/blob/master/specification/VRMC_vrm-1.0/schema/VRMC_vrm.expressions.schema.json
        u8"/extensions/VRMC_vrm/expressions/preset",
        { {
          { u8"happy", u8"😆" },
          { u8"angry", u8"😠" },
          { u8"sad", u8"😥" },
          { u8"relaxed", u8"🙂" },
          { u8"surprised", u8"😲" },
          { u8"aa", u8"👄" },
          { u8"ih", u8"👄" },
          { u8"ou", u8"👄" },
          { u8"ee", u8"👄" },
          { u8"oh", u8"👄" },
          { u8"blink", u8"😉" },
          { u8"blinkLeft", u8"😉" },
          { u8"blinkRight", u8"😉" },
          { u8"lookUp", u8"👀" },
          { u8"lookDown", u8"👀" },
          { u8"lookLeft", u8"👀" },
          { u8"lookRight", u8"👀" },
          { u8"neutral", u8"😶" },
        } },
      },
      // glTF
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/asset.schema.json
        u8"/asset",
        { {
          { u8"version", u8"📄", { {}, U8Q("") }, JsonPropFlags::Required },
          { u8"minVersion", u8"📄", { {}, U8Q("") } },
          { u8"copyright", u8"📄", { {}, U8Q("") } },
          { u8"generator", u8"📄", { {}, U8Q("") } },
        } },
      },
      // buffer/bufferView/accessor
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/buffer.schema.json
        u8"/buffers/*",
        { {
          {
            u8"byteLength",
            u8"🔢",
            {},
            JsonPropFlags::Required,
          },
          {
            u8"uri",
            u8"📄",
            {},
          },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/bufferView.schema.json
        u8"/bufferViews/*",
        { {
          { u8"buffer", u8"🆔", {}, JsonPropFlags::Required },
          { u8"byteLength", u8"📄", {}, JsonPropFlags::Required },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/accessor.schema.json
        u8"/accessors/*",
        { {
          { u8"componentType", u8"🔢", {}, JsonPropFlags::Required },
          { u8"type", u8"📄", {}, JsonPropFlags::Required },
          { u8"count", u8"🔢", {}, JsonPropFlags::Required },
        } },
      },
      // image/sampler/texture/material
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
          { u8"extensions", u8"⭐", { {}, u8"{}" } },
          { u8"pbrMetallicRoughness", u8"💎", { {}, u8"{}" } },
          { u8"normalTexture", u8"🖼", { {}, u8"{}" } },
          { u8"occlusionTexture", u8"🖼", { {}, u8"{}" } },
          { u8"emissiveTexture", u8"🖼", { {}, u8"{}" } },
          { u8"emissiveFactor", u8"🎨", { RgbPicker{}, u8"[0,0,0]" } },
          { u8"alphaMode", u8"📄" },
          { u8"alphaCutoff", u8"🎚️", { FloatSlider{}, u8"0.5" } },
          { u8"doubleSided", u8"✅", { {}, u8"false" } },
        } },
      },
      {
        u8"/materials/*/extensions",
        { {
          { u8"KHR_materials_unlit", u8"🏛️", { {}, u8"{}" } },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/material.pbrMetallicRoughness.schema.json
        u8"/materials/*/pbrMetallicRoughness",
        { {
          { u8"baseColorFactor", u8"🎨", { RgbaPicker{} } },
          { u8"baseColorTexture", u8"🖼" },
          {
            u8"metallicFactor",
            u8"🎚️",
            { FloatSlider{ .Default = 1.0f } },
          },
          {
            u8"roughnessFactor",
            u8"🎚️",
            { FloatSlider{ .Default = 1.0f } },
          },
          { u8"metallicRoughnessTexture", u8"🖼" },
        } },
      },
      // mesh/skin
      {
        // https
        // ://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/mesh.primitive.schema.json
        u8"/meshes/*/primitives/*",
        { {
          { u8"attributes", u8"📄", {}, JsonPropFlags::Required },
          { u8"indices", u8"📄", {} },
          { u8"material", u8"🆔" },
        } },
      },
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/mesh.schema.json
        u8"/meshes/*",
        { {
          { u8"primitives", u8"[]", {}, JsonPropFlags::Required },
          { u8"weights", u8"[]" },
        } },
      },
      // node
      {
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/node.schema.json
        u8"/nodes/*",
        { {
          { u8"mesh", u8"🆔" },
        } },
      },
    })
{
}

void
JsonGui::ClearCache(const std::u8string& jsonpath)
{
  if (jsonpath.size()) {
    // clear all descendants
    for (auto it = m_cacheMap.begin(); it != m_cacheMap.end();) {
      if (it->first.starts_with(jsonpath)) {
        // clear all descendants
        it = m_cacheMap.erase(it);
      } else {
        ++it;
      }
    }

    gltfjson::JsonPath path(jsonpath);
    auto [childOfRoot, i] = path.GetChildOfRootIndex();
    if (childOfRoot == u8"materials") {
      glr::ReleaseMaterial(i);
    }
  } else {
    m_cacheMap.clear();
    glr::Release();
  }
}

void
JsonGui::SetScene(const std::shared_ptr<libvrm::GltfRoot>& root)
{
  m_root = root;
  ClearCache();
}

std::tuple<bool, EditorResult>
JsonGui::Enter(const gltfjson::tree::NodePtr& item,
               const std::u8string& jsonpath,
               const JsonProp& prop)
{
  auto result = EditorResult::None;

  static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                         ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                         ImGuiTreeNodeFlags_SpanAvailWidth;
  ImGuiTreeNodeFlags node_flags = base_flags;
  bool is_leaf = !item;
  if (item) {
    if (auto array = item->Array()) {
      if (array->size() == 0) {
        is_leaf = true;
      }
    }
  }
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
    auto inserted = m_cacheMap.insert(
      { jsonpath, { prop.Label(), prop.Value.TextOrDeault(item) } });
    cache = &inserted.first->second;
    cache->Editor = prop.Value.EditorOrDefault();
  }
  auto node_open = ImGui::TreeNodeEx(
    (void*)(intptr_t)id, node_flags, "%s", (const char*)cache->Label.data());
  ImGui::PopStyleColor(push);

  ImGui::PushID((const char*)jsonpath.c_str());

  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    m_jsonpath = jsonpath;
  }

  // 1
  ImGui::TableNextColumn();

  if (item) {
    if (isSelected) {
      if (cache->Editor(m_root->m_gltf->m_json, m_root->m_bin, item)) {
        result = EditorResult::Updated;
      }
    } else {
      assert(cache->Value.size());
      ImGui::TextUnformatted((const char*)cache->Value.c_str());
    }
  } else {
    if (cache->Value.size()) {
      ImGui::BeginDisabled(true);
      ImGui::TextUnformatted((const char*)cache->Value.c_str());
      ImGui::EndDisabled();
    } else {
      ImGui::PushStyleColor(ImGuiCol_Text, grapho::imcolor::orange);
      ImGui::TextUnformatted("no default");
      ImGui::PopStyleColor();
    }
  }

  // 2 add/remove
  ImGui::TableNextColumn();
  if (Has(prop.Flags, JsonPropFlags::Unknown)) {
    if (ImGui::Button("-##unknown")) {
      result = EditorResult::Removed;
    }
  } else if (Has(prop.Flags, JsonPropFlags::Required)) {
    ImGui::TextUnformatted("📍");
  } else if (item) {
    if (item->Array()) {
      if (ImGui::Button("+##array_append")) {
        result = EditorResult::ArrayAppended;
      }
    } else {
      if (ImGui::Button("-##removed")) {
        result = EditorResult::Removed;
      }
    }
  } else {
    if (ImGui::Button("+##key_created")) {
      result = EditorResult::KeyCreated;
    }
  }
  ImGui::PopID();

  return { node_open && !is_leaf, result };
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

EditorResult
JsonGui::Traverse(const gltfjson::tree::NodePtr& item,
                  std::u8string& jsonpath,
                  const JsonProp& prop)
{
  auto [isOpen, result] = Enter(item, jsonpath, prop);
  switch (result) {
    case EditorResult::None:
      break;
    case EditorResult::Updated:
      ClearCache(jsonpath);
      break;
    case EditorResult::KeyCreated:
      ClearCache(jsonpath);
      break;
    case EditorResult::ArrayAppended:
      break;
    case EditorResult::Removed:
      ClearCache(jsonpath);
      break;
  }
  if (isOpen) {
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
            auto child_result = Traverse(child, jsonpath, prop);
            if (child_result == EditorResult::Removed) {
              PLOG_DEBUG << "prop.key remove: " << gltfjson::from_u8(jsonpath);
              item->Remove(prop.Key);
              result = EditorResult::Updated;
            }
          } else {
            auto child_result = Traverse(nullptr, jsonpath, prop);
            if (child_result == EditorResult::KeyCreated) {
              gltfjson::tree::Parser parser(prop.Value.DefaultJson);
              if (auto new_child = parser.ParseExpected()) {
                object->insert({ prop.Key, *new_child });
                result = EditorResult::Updated;
              } else {
                PLOG_ERROR << gltfjson::from_u8(new_child.error());
              }
            }
          }
          jsonpath.resize(size);
        }
      }

      for (auto it = object->begin(); it != object->end();) {
        auto child_result = EditorResult::None;
        jsonpath += it->first;
        if (used.find(it->first) == used.end()) {
          child_result = Traverse(
            it->second,
            jsonpath,
            { jsonpath.substr(size), u8"❔", {}, JsonPropFlags::Unknown });
        }

        if (child_result == EditorResult::Removed) {
          PLOG_DEBUG << "unknown key remove: " << gltfjson::from_u8(jsonpath);
          it = object->erase(it);
          result = EditorResult::Updated;
        } else {
          ++it;
        }
        jsonpath.resize(size);
      }
    } else if (auto array = item->Array()) {
      int i = 0;
      std::optional<int> removed;
      for (auto& child : *array) {
        gltfjson::tree::concat_int(jsonpath, i);
        auto child_result = Traverse(
          child,
          jsonpath,
          { jsonpath.substr(size), prop.Icon, {}, JsonPropFlags::ArrayChild });
        if (child_result == EditorResult::Removed) {
          removed = i;
        }
        jsonpath.resize(size);
        ++i;
      }
      if (removed) {
        PLOG_DEBUG << "array remove: " << gltfjson::from_u8(jsonpath) << " at "
                   << *removed;
        result = EditorResult::Updated;
        array->erase(std::next(array->begin(), *removed));
      }
    }
    ImGui::TreePop();
  }
  return result;
}
