#include "json_gui.h"
#include "../docks/gui.h"
#include "json_widgets.h"
#include "jsonschema/extensions.h"
#include "jsonschema/gltf.h"
#include "jsonschema/vrm0.h"
#include "jsonschema/vrm1.h"
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

JsonGui::JsonGui()
{
  for (auto& kv : jsonschema::VRMC_vrm()) {
    m_definitionMap.m_map.push_back(kv);
  }
  for (auto& kv : jsonschema::VRM()) {
    m_definitionMap.m_map.push_back(kv);
  }
  for (auto& kv : jsonschema::Gltf()) {
    m_definitionMap.m_map.push_back(kv);
  }
  for (auto& kv : jsonschema::Extensions()) {
    m_definitionMap.m_map.push_back(kv);
  }
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
    } else if (item->Object()) {
    } else {
      is_leaf = true;
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

  // 0: tree
  ImGui::TableNextColumn();
  if (ShouldOpen(jsonpath)) {
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
  }

  int push = 0;
  if (item) {
    if (Has(prop.Flags, JsonPropFlags::Unknown)) {
      ImGui::PushStyleColor(ImGuiCol_Text, grapho::imcolor::orange);
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
      { jsonpath, { prop.Name.Label(), prop.Value.TextOrDeault(item) } });
    cache = &inserted.first->second;
    cache->Editor = prop.Value.EditorOrDefault();
    if (prop.Tag) {
      cache->ShowTag = prop.Tag(m_root->m_gltf->m_json, m_root->m_bin, item);
    }
  }
  auto node_open = ImGui::TreeNodeEx(
    (void*)(intptr_t)id, node_flags, "%s", (const char*)cache->Label.data());
  ImGui::PopStyleColor(push);

  ImGui::PushID((const char*)jsonpath.c_str());

  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    m_jsonpath = jsonpath;
  }

  // 1: tag
  ImGui::TableNextColumn();
  if (cache->ShowTag) {
    cache->ShowTag();
  }

  // 2: value
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

  // 3 add/remove
  ImGui::TableNextColumn();
  if (jsonpath == u8"/") {
  } else if (Has(prop.Flags, JsonPropFlags::Unknown)) {
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
JsonGui::ShowSelector()
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

  std::array<const char*, 4> cols = {
    "Name",
    "Tag",
    "Value",
    "✅",
  };

  // auto size = ImGui::GetContentRegionAvail();

  if (grapho::imgui::BeginTableColumns("##JsonGui::ShowSelector", cols)) {

    // tree
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, Gui::Instance().Indent());

    std::u8string jsonpath(u8"/");
    Traverse(m_root->m_gltf->m_json,
             jsonpath,
             JsonProp{ { u8"", u8"glTF" }, {}, JsonPropFlags::Unknown });

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
      //
      // object
      //
      std::unordered_set<std::u8string> used;
      // used.clear();
      if (auto definition = m_definitionMap.Match(jsonpath)) {
        for (auto prop : definition->Props) {
          jsonpath += prop.Name.Key;
          used.insert(prop.Name.Key);
          if (auto child = item->Get(prop.Name.Key)) {
            auto child_result = Traverse(child, jsonpath, prop);
            if (child_result == EditorResult::Removed) {
              PLOG_DEBUG << "prop.key remove: " << gltfjson::from_u8(jsonpath);
              item->Remove(prop.Name.Key);
              result = EditorResult::Updated;
            }
          } else {
            auto child_result = Traverse(nullptr, jsonpath, prop);
            if (child_result == EditorResult::KeyCreated) {
              gltfjson::tree::Parser parser(prop.Value.DefaultJson);
              if (auto new_child = parser.ParseExpected()) {
                object->insert({ prop.Name.Key, *new_child });
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
            { { u8"❔", jsonpath.substr(size) }, {}, JsonPropFlags::Unknown });
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
      //
      // array
      //
      bool hasProp = false;
      JsonProp child_prop;
      if (auto definition = m_definitionMap.Match(jsonpath)) {
        if (definition->Props.size()) {
          child_prop = definition->Props.front();
          hasProp = true;
        }
      }
      if (!hasProp) {
        child_prop = {
          { prop.Name.Icon },
          {},
          JsonPropFlags::ArrayChild,
        };
      }

      int i = 0;
      std::optional<int> removed;
      for (auto& child : *array) {
        gltfjson::tree::concat_int(jsonpath, i);
        child_prop.Name.Key = jsonpath.substr(size);
        auto child_result = Traverse(child, jsonpath, child_prop);
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
