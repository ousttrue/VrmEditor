#include "json_gui.h"
// #include "json_gui_accessor.h"
// #include "json_gui_images.h"
// #include "json_gui_material.h"
// #include "json_gui_mesh.h"
// #include "json_gui_node.h"
// #include "json_gui_skin.h"
// #include "json_gui_vrm0.h"
// #include "json_gui_vrm1.h"
#include <charconv>
#include <imgui.h>
#include <imgui_internal.h>
#include <ranges>
#include <sstream>
#include <string_view>

static std::u8string_view
GetIcon(const gltfjson::JsonPath& path)
{
  if (path.Size() == 2) {
    auto kind = path[1];
    if (kind == u8"extensions") {
      return u8" ";
    } else if (kind == u8"images" || kind == u8"textures" ||
               kind == u8"samplers") {
      return u8" ";
    } else if (kind == u8"bufferViews" || kind == u8"buffers") {
      return u8" ";
    } else if (kind == u8"accessors") {
      return u8" ";
    } else if (kind == u8"meshes" || kind == u8"skins") {
      return u8"󰕣 ";
    } else if (kind == u8"materials") {
      return u8" ";
    } else if (kind == u8"nodes" || kind == u8"scenes" || kind == u8"scene") {
      return u8"󰵉 ";
    } else if (kind == u8"animations") {
      return u8" ";
    } else if (kind == u8"asset") {
      return u8" ";
    } else if (kind == u8"extensionsUsed") {
      return u8" ";
    }
  }

  return u8"";
}

// static std::u8string
// LabelDefault(const gltfjson::tree::NodePtr& item, std::u8string_view
// jsonpath)
// {
//   std::stringstream ss;
//   // std::u8string key{ jsonpath.begin(), jsonpath.end() };
//   auto path = gltfjson::JsonPath(jsonpath);
//   // if (path.Size() == 2) {
//   //   auto kind = path[1];
//   //   if (kind == u8"extensions") {
//   //     ss << " ";
//   //   } else if (kind == u8"images" || kind == u8"textures" ||
//   //              kind == u8"samplers") {
//   //     ss << " ";
//   //   } else if (kind == u8"bufferViews" || kind == u8"buffers") {
//   //     ss << " ";
//   //   } else if (kind == u8"accessors") {
//   //     ss << " ";
//   //   } else if (kind == u8"meshes" || kind == u8"skins") {
//   //     ss << "󰕣 ";
//   //   } else if (kind == u8"materials") {
//   //     ss << " ";
//   //   } else if (kind == u8"nodes" || kind == u8"scenes" || kind ==
//   u8"scene") {
//   //     ss << "󰵉 ";
//   //   } else if (kind == u8"animations") {
//   //     ss << " ";
//   //   } else if (kind == u8"asset") {
//   //     ss << " ";
//   //   } else if (kind == u8"extensionsUsed") {
//   //     ss << " ";
//   //   }
//   // }
//
//   // auto item = scene->m_gltf.Json.at(nlohmann::json::json_pointer(key));
//   auto name = path.Back();
//   // std::string name{ _name.begin(), _name.end() };
//   if (auto object = item->Object()) {
//     auto found = object->find(u8"name");
//     if (found != object->end()) {
//       ss << gltfjson::tree::from_u8(name) << ": "
//          << gltfjson::tree::from_u8(found->second->U8String());
//     } else {
//       ss << gltfjson::tree::from_u8(name) << ": object";
//     }
//   } else if (auto array = item->Array()) {
//     ss << gltfjson::tree::from_u8(name) << ": [" << array->size() << "]";
//   } else {
//     ss << gltfjson::tree::from_u8(name) << ": " << *item;
//   }
//   return (const char8_t*)ss.str().c_str();
// }

LabelCacheManager::LabelCacheManager()
  : m_labelFactories({
      //
      // { "/images", LabelArray },
    })
{
}

static std::u8string_view
GetLastName(std::u8string_view src)
{
  auto pos = src.rfind('/');
  if (pos != std::string::npos) {
    return src.substr(pos + 1);
  } else {
    return src;
  }
}

const LabelCache&
LabelCacheManager::Get(const gltfjson::tree::NodePtr& item,
                       std::u8string_view jsonpath)
{
  auto found = m_labelCache.find({ jsonpath.begin(), jsonpath.end() });
  if (found != m_labelCache.end()) {
    return found->second;
  }

  LabelCache label;
  auto path = gltfjson::JsonPath(jsonpath);
  auto icon = GetIcon(path);
  label.Key += gltfjson::tree::from_u8(icon);
  label.Key += gltfjson::tree::from_u8(GetLastName(jsonpath));
  if (auto factory = MatchLabel(jsonpath)) {
    label.Value = (*factory)(m_scene, jsonpath);
  } else {
    std::stringstream ss;
    ss << *item;
    label.Value = ss.str();
  }
  auto inserted =
    m_labelCache.insert({ { jsonpath.begin(), jsonpath.end() }, label });
  return inserted.first->second;
}

JsonGui::JsonGui()
  : //
  m_guiFactories({
    //
    // { "/accessors", JsonGuiAccessorList },
    // { "/accessors/*", JsonGuiAccessor },
    // { "/images", JsonGuiImageList },
    // { "/materials", JsonGuiMaterialList },
    // { "/meshes", JsonGuiMeshList },
    // { "/meshes/*", JsonGuiMesh },
    // { "/meshes/*/primitives/*/attributes/POSITION", JsonGuiAccessor },
    // { "/meshes/*/primitives/*/attributes/NORMAL", JsonGuiAccessor },
    // { "/meshes/*/primitives/*/attributes/TEXCOORD_0", JsonGuiAccessor },
    // { "/meshes/*/primitives/*/attributes/JOINTS_0", JsonGuiAccessor },
    // { "/meshes/*/primitives/*/attributes/WEIGHTS_0", JsonGuiAccessor },
    // { "/skins", JsonGuiSkinList },
    // { "/skins/*/inverseBindMatrices", JsonGuiAccessor },
    // { "/nodes", JsonGuiNodeList },
    // { "/extensions/VRM/secondaryAnimation/colliderGroups/*/colliders",
    //   JsonGuiVrm0ColliderList },
    // { "/extensions/VRM/secondaryAnimation/boneGroups", JsonGuiVrm0SpringList
    // },
    // { "/extensions/VRMC_springBone/springs/*/joints", JsonGuiVrm1SpringJoints
    // },
    // { "/extensions/VRMC_springBone/colliders", JsonGuiVrm1SpringColliders },
    //
  })
//
{
}

bool
JsonGui::Enter(const gltfjson::tree::NodePtr& item, std::u8string_view jsonpath)
{
  static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                         ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                         ImGuiTreeNodeFlags_SpanAvailWidth;
  ImGuiTreeNodeFlags node_flags = base_flags;
  auto is_leaf = !item->Object() && !item->Array();
  if (is_leaf) {
    node_flags |=
      ImGuiTreeNodeFlags_Leaf |
      ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
  }
  if (jsonpath == m_selected) {
    node_flags |= ImGuiTreeNodeFlags_Selected;
  }

  auto& label = m_label.Get(item, jsonpath);

  ImGui::TableNextRow();

  // 0
  ImGui::TableNextColumn();
  bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)item.get(),
                                     node_flags,
                                     "%s",
                                     (const char*)label.Key.c_str());

  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    m_selected = jsonpath;
    m_cache = {};
  }

  // 1
  ImGui::TableNextColumn();
  ImGui::TextUnformatted((const char*)label.Value.c_str());

  return node_open && !is_leaf;
}

// https://github.com/ocornut/imgui/issues/319
static bool
Splitter(bool split_vertically,
         float thickness,
         float* size1,
         float* size2,
         float min_size1,
         float min_size2,
         float splitter_long_axis_size = -1.0f)
{
  using namespace ImGui;
  ImGuiContext& g = *GImGui;
  ImGuiWindow* window = g.CurrentWindow;
  ImGuiID id = window->GetID("##Splitter");
  ImRect bb;
  bb.Min = window->DC.CursorPos +
           (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
  bb.Max = bb.Min + CalcItemSize(split_vertically
                                   ? ImVec2(thickness, splitter_long_axis_size)
                                   : ImVec2(splitter_long_axis_size, thickness),
                                 0.0f,
                                 0.0f);
  return SplitterBehavior(bb,
                          id,
                          split_vertically ? ImGuiAxis_X : ImGuiAxis_Y,
                          size1,
                          size2,
                          min_size1,
                          min_size2,
                          0.0f);
}

void
JsonGui::ShowSelector(float indent)
{
  auto enter = [this](const gltfjson::tree::NodePtr& item,
                      std::u8string_view jsonpath) {
    return Enter(item, jsonpath);
  };
  auto leave = []() { ImGui::TreePop(); };

  static ImGuiTableFlags flags =
    ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH |
    ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
    ImGuiTableFlags_NoBordersInBody;

  if (ImGui::BeginTable("3ways", 2, flags)) {
    // The first column will use the default _WidthStretch when ScrollX is Off
    // and _WidthFixed when ScrollX is On
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
    ImGui::TableSetupColumn(
      "Value", ImGuiTableColumnFlags_WidthFixed, 20 * 12.0f);
    ImGui::TableHeadersRow();

    {
      // tree
      ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, indent);
      gltfjson::tree::TraverseJson(enter, leave, m_scene->m_gltf->m_json);
      ImGui::PopStyleVar();
    }

    ImGui::EndTable();
  }
}

void
JsonGui::ShowSelected()
{
  ImGui::TextUnformatted((const char*)m_selected.c_str());
  if (!m_cache) {
    if (auto mached = MatchGui(m_selected)) {
      m_cache = (*mached)(m_scene, m_selected);
    } else {
      m_cache = []() {};
    }
  }
  m_cache();
}
