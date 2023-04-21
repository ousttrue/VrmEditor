#include "json_gui.h"
#include "json_gui_accessor.h"
#include "json_gui_images.h"
#include "json_gui_material.h"
#include "json_gui_mesh.h"
#include "json_gui_node.h"
#include "json_gui_skin.h"
#include "json_gui_vrm0.h"
#include "json_gui_vrm1.h"
#include <charconv>
#include <imgui.h>
#include <imgui_internal.h>
#include <ranges>
#include <string_view>

static std::string
LabelDefault(const std::shared_ptr<libvrm::gltf::Scene>& scene,
             std::string_view jsonpath)
{
  std::stringstream ss;
  std::string key{ jsonpath.begin(), jsonpath.end() };
  auto item = scene->m_gltf.Json.at(nlohmann::json::json_pointer(key));
  auto path = libvrm::JsonPath(jsonpath);
  auto name = path.Back();
  if (path.Size() == 2) {
    auto kind = path[1];
    if (kind == "extensions") {
      ss << " ";
    } else if (kind == "images" || kind == "textures" || kind == "samplers") {
      ss << " ";
    } else if (kind == "bufferViews" || kind == "buffers") {
      ss << " ";
    } else if (kind == "accessors") {
      ss << " ";
    } else if (kind == "meshes" || kind == "skins") {
      ss << "󰕣 ";
    } else if (kind == "materials") {
      ss << " ";
    } else if (kind == "nodes" || kind == "scenes" || kind == "scene") {
      ss << "󰵉 ";
    } else if (kind == "animations") {
      ss << " ";
    } else if (kind == "asset") {
      ss << " ";
    } else if (kind == "extensionsUsed") {
      ss << " ";
    }
  }

  if (item.is_object()) {
    if (item.find("name") != item.end()) {
      ss << name << ": " << (std::string_view)item.at("name");
    } else {
      ss << name << ": object";
    }
  } else if (item.is_array()) {
    ss << name << ": [" << item.size() << "]";
  } else {
    ss << name << ": " << item.dump();
  }
  return ss.str();
}

JsonGui::JsonGui(const std::shared_ptr<libvrm::gltf::Scene>& scene)
  : //
  m_guiFactories({
    //
    { "/accessors", JsonGuiAccessorList },
    { "/accessors/*", JsonGuiAccessor },
    { "/images", JsonGuiImageList },
    { "/materials", JsonGuiMaterialList },
    { "/meshes", JsonGuiMeshList },
    { "/meshes/*", JsonGuiMesh },
    { "/meshes/*/primitives/*/attributes/POSITION", JsonGuiAccessor },
    { "/meshes/*/primitives/*/attributes/NORMAL", JsonGuiAccessor },
    { "/meshes/*/primitives/*/attributes/TEXCOORD_0", JsonGuiAccessor },
    { "/meshes/*/primitives/*/attributes/JOINTS_0", JsonGuiAccessor },
    { "/meshes/*/primitives/*/attributes/WEIGHTS_0", JsonGuiAccessor },
    { "/skins", JsonGuiSkinList },
    { "/skins/*/inverseBindMatrices", JsonGuiAccessor },
    { "/nodes", JsonGuiNodeList },
    {
      "/extensions/VRM/secondaryAnimation/boneGroups",
      JsonGuiVrm0SpringList,
    },
    { "/extensions/VRMC_springBone/springs/*/joints", JsonGuiVrm1SpringJoints },
    //
  })
  //
  , m_labelFactories({
      //
      // { "/images", LabelArray },
    })
  , m_scene(scene)
{
}

bool
JsonGui::Enter(nlohmann::json& item, std::string_view jsonpath)
{
  static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                         ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                         ImGuiTreeNodeFlags_SpanAvailWidth;
  ImGuiTreeNodeFlags node_flags = base_flags;
  auto is_leaf = !item.is_object() && !item.is_array();
  if (is_leaf) {
    node_flags |=
      ImGuiTreeNodeFlags_Leaf |
      ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
  }
  if (jsonpath == m_selected) {
    node_flags |= ImGuiTreeNodeFlags_Selected;
  }

  std::string label = "not found";
  auto found = m_labelCache.find({ jsonpath.begin(), jsonpath.end() });
  if (found != m_labelCache.end()) {
    label = found->second;
  } else {
    if (auto factory = MatchLabel(jsonpath)) {
      label = (*factory)(m_scene, jsonpath);
    } else {
      label = LabelDefault(m_scene, jsonpath);
    }
    m_labelCache.insert({ { jsonpath.begin(), jsonpath.end() }, label });
  }

  bool node_open =
    ImGui::TreeNodeEx((void*)(intptr_t)&item, node_flags, "%s", label.c_str());

  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    m_selected = jsonpath;
    if (auto mached = MatchGui(m_selected)) {
      m_cache = (*mached)(m_scene, m_selected);
    } else {
      m_cache = []() {};
    }
  }

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
JsonGui::Show(float indent)
{
  auto enter = [this](nlohmann::json& item, std::string_view jsonpath) {
    return Enter(item, jsonpath);
  };
  auto leave = []() { ImGui::TreePop(); };

  // auto size = ImGui::GetCurrentWindow()->Size;
  auto size = ImGui::GetContentRegionAvail();
  float s = size.y - m_f - 5;
  // ImGui::Text("%f, %f: %f; %f", size.x, size.y, f, s);
  ::Splitter(false, 5, &m_f, &s, 8, 8);
  if (ImGui::BeginChild("##split-first", { size.x, m_f })) {
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, indent);
    m_scene->TraverseJson(enter, leave);
    ImGui::PopStyleVar();
  }
  ImGui::EndChild();

  if (ImGui::BeginChild("##split-second", { size.x, s })) {
    ImGui::TextUnformatted(m_selected.c_str());
    m_cache();
  }
  ImGui::EndChild();
}
