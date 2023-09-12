#include "json_gui.h"
// #include "../docks/gui.h"
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
  // PLOG_DEBUG << "ClearCache: " << gltfjson::from_u8(jsonpath);
  if (jsonpath.size()) {
    // clear all descendants
    for (auto it = m_cacheMap.begin(); it != m_cacheMap.end();) {
      if (it->first.starts_with(jsonpath)) {
        // clear all descendants
        // PLOG_DEBUG << "  ClearCache: " << gltfjson::from_u8(it->first);
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

  std::array<const char*, 4> cols = {
    "Name",
    "Tag",
    "Value",
    "✅",
  };

  // auto size = ImGui::GetContentRegionAvail();

  if (grapho::imgui::BeginTableColumns("##JsonGui::ShowSelector", cols)) {

    // tree
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, indent);

    std::u8string jsonpath(u8"/");
    Traverse(
      m_root->m_gltf->m_json, jsonpath, JsonProp{ { u8"", u8"glTF" }, {} });

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
  if (isOpen) {
    gltfjson::tree::AddDelimiter(jsonpath);
    auto size = jsonpath.size();
    if (auto object =
          std::dynamic_pointer_cast<gltfjson::tree::ObjectNode>(item)) {
      //
      // object
      //
      std::unordered_set<std::u8string> used;
      if (auto definition = m_definitionMap.Match(jsonpath)) {
        for (auto prop : definition->Props) {
          jsonpath += prop.Name.Key;
          used.insert(prop.Name.Key);
          EditorResult child_result = {};
          gltfjson::tree::NodePtr child = {};
          if ((child = item->Get(prop.Name.Key))) {
            child_result = Traverse(child, jsonpath, prop);
          } else {
            child_result = Traverse(nullptr, jsonpath, prop);
          }
          if (OnEdit(item, child, jsonpath, prop, child_result)) {
            result = EditorResult::Updated;
          }
          jsonpath.resize(size);
        }
      }
      std::function<void()> removeAfter;
      for (auto& it : object->Value) {
        auto child_result = EditorResult::None;
        jsonpath += it.first;
        if (used.find(it.first) == used.end()) {
          JsonProp prop{ { u8"❔", jsonpath.substr(size) } };
          child_result = Traverse(it.second, jsonpath, prop);
          if (child_result != EditorResult::None) {
            removeAfter = [=, jp = jsonpath, p = &result]() {
              if (OnEdit(item, it.second, jp, prop, child_result)) {
                *p = EditorResult::Updated;
              }
            };
          }
        }
        jsonpath.resize(size);
      }
      if (removeAfter) {
        removeAfter();
      }
    } else if (auto array =
                 std::dynamic_pointer_cast<gltfjson::tree::ArrayNode>(item)) {
      //
      // array
      //
      bool hasProp = false;
      JsonProp child_prop{};
      if (auto definition = m_definitionMap.Match(jsonpath)) {
        if (definition->Props.size()) {
          child_prop = definition->Props.front();
          hasProp = true;
        }
      }
      if (!hasProp) {
        child_prop = { { prop.Name.Icon }, {} };
      }
      child_prop.Flags = child_prop.Flags | JsonPropFlags::NoRemove;

      int i = 0;
      std::function<void()> removeAfter;
      for (auto& child : array->Value) {
        gltfjson::tree::concat_int(jsonpath, i);
        child_prop.Name.Key = jsonpath.substr(size);
        auto child_result = Traverse(child, jsonpath, child_prop);
        if (child_result != EditorResult::None) {
          removeAfter = [=, jp = jsonpath, p = &result]() mutable {
            if (OnEdit(item, child, jp, child_prop, child_result)) {
              *p = EditorResult::Updated;
            }
          };
        }
        jsonpath.resize(size);
        ++i;
      }

      if (Has(prop.Flags, JsonPropFlags::ArrayAdd)) {
        // add array child
        gltfjson::tree::concat_int(jsonpath, i);
        child_prop.Name.Key = jsonpath.substr(size);
        auto child_result = Traverse(nullptr, jsonpath, child_prop);
        if (OnEdit(item, nullptr, jsonpath, child_prop, child_result)) {
          result = EditorResult::Updated;
        }
        jsonpath.resize(size);
      }

      if (removeAfter) {
        removeAfter();
      }
    }
    // remove delimiter
    jsonpath.pop_back();
    ImGui::TreePop();
  }
  return result;
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
    if (std::dynamic_pointer_cast<gltfjson::tree::ArrayNode>(item)) {
    } else if (std::dynamic_pointer_cast<gltfjson::tree::ObjectNode>(item)) {
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
    // if (Has(prop.Flags, JsonPropFlags::Unknown)) {
    //   ImGui::PushStyleColor(ImGuiCol_Text, grapho::imcolor::orange);
    //   ++push;
    // }
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
      ImGui::TextUnformatted("no definition");
      ImGui::PopStyleColor();
    }
  }

  // 3 add/remove
  ImGui::TableNextColumn();
  if (jsonpath == u8"/") {
    // } else if (Has(prop.Flags, JsonPropFlags::Unknown)) {
    //   if (ImGui::Button("-##unknown")) {
    //     result = EditorResult::Removed;
    //   }
  } else if (Has(prop.Flags, JsonPropFlags::Required)) {
    ImGui::TextUnformatted("📍");
  } else if (item) {
    if (!Has(prop.Flags, JsonPropFlags::NoRemove)) {
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

bool
JsonGui::OnEdit(const gltfjson::tree::NodePtr& parent,
                const gltfjson::tree::NodePtr& item,
                const std::u8string& jsonpath,
                const JsonProp& prop,
                EditorResult result)
{
  switch (result) {
    case EditorResult::None:
      break;

    case EditorResult::Updated:
      ClearCache(jsonpath);
      break;

    case EditorResult::KeyCreated: {
      assert(!item);
      ClearCache(jsonpath);
      gltfjson::tree::Parser parser(prop.Value.DefaultJson);
      if (auto new_child = parser.Parse()) {
        if (auto object =
              std::dynamic_pointer_cast<gltfjson::tree::ObjectNode>(parent)) {
          object->Value.insert({ prop.Name.Key, new_child });
          return true;
        } else if (auto array =
                     std::dynamic_pointer_cast<gltfjson::tree::ArrayNode>(
                       parent)) {
          array->Value.push_back(new_child);
          return true;
        }
      } else {
        // PLOG_ERROR << gltfjson::from_u8(new_child.error());
        return false;
      }
      break;
    }

    case EditorResult::Removed:
      ClearCache(jsonpath);
      if (auto array =
            std::dynamic_pointer_cast<gltfjson::tree::ArrayNode>(parent)) {
        int i = 0;
        for (auto it = array->Value.begin(); it != array->Value.end();
             ++it, ++i) {
          if (*it == item) {
            PLOG_DEBUG << "array remove: " << gltfjson::from_u8(jsonpath)
                       << " at" << i;
            array->Value.erase(it);
            return true;
          }
        }
      } else if (auto object =
                   std::dynamic_pointer_cast<gltfjson::tree::ObjectNode>(
                     parent)) {
        for (auto it = object->Value.begin(); it != object->Value.end(); ++it) {
          if (it->second == item) {
            PLOG_DEBUG << "prop.key remove: " << gltfjson::from_u8(jsonpath);
            object->Value.erase(it);
            return true;
          }
        }
      }
      break;
  }

  return false;
}
