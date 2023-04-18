#include "json_dock_impl.h"
#include <charconv>
#include <imgui.h>
#include <imgui_internal.h>
#include <ranges>
#include <string_view>


JsonDockImpl::JsonDockImpl(const std::shared_ptr<libvrm::gltf::Scene>& scene)
  : m_scene(scene)
{
}

bool
JsonDockImpl::Enter(nlohmann::json& item, std::string_view jsonpath)
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
  if (jsonpath == m_selected.Str) {
    node_flags |= ImGuiTreeNodeFlags_Selected;
  }

  m_ss.str("");

  bool extension = false;
  std::string_view kind;
  std::string_view key;
  int i = 0;
  for (auto jp : jsonpath | std::views::split('.')) {
    key = std::string_view(jp);
    if (i == 0) {
      kind = key;
    }
    ++i;
    if (key == "extensions" || key == "extrans") {
      extension = true;
      break;
    }
  }

  if (extension) {
    m_ss << " ";
  } else if (kind == "images" || kind == "textures" || kind == "samplers") {
    m_ss << " ";
  } else if (kind == "bufferViews" || kind == "buffers") {
    m_ss << " ";
  } else if (kind == "accessors") {
    m_ss << " ";
  } else if (kind == "meshes" || kind == "skins") {
    m_ss << "󰕣 ";
  } else if (kind == "materials") {
    m_ss << " ";
  } else if (kind == "nodes" || kind == "scenes" || kind == "scene") {
    m_ss << "󰵉 ";
  } else if (kind == "animations") {
    m_ss << " ";
  } else if (kind == "asset") {
    m_ss << " ";
  } else if (kind == "extensionsUsed") {
    m_ss << " ";
  }

  if (item.is_object()) {
    if (item.find("name") != item.end()) {
      m_ss << key << ": " << (std::string_view)item.at("name");
    } else {
      m_ss << key << ": object";
    }
  } else if (item.is_array()) {
    m_ss << key << ": [" << item.size() << "]";
  } else {
    m_ss << key << ": " << item.dump();
  }
  auto label = m_ss.str();
  bool node_open =
    ImGui::TreeNodeEx((void*)(intptr_t)&item, node_flags, "%s", label.c_str());

  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    m_selected = libvrm::JsonPath(jsonpath);
    m_cache = {};
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
JsonDockImpl::Show(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                   float indent)
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
    scene->TraverseJson(enter, leave);
    ImGui::PopStyleVar();
  }
  ImGui::EndChild();

  if (ImGui::BeginChild("##split-second", { size.x, s })) {
    if (!m_cache) {
      m_cache = CreateGui();
    }
    m_cache();
  }
  ImGui::EndChild();
}

std::function<void()>
JsonDockImpl::CreateGui()
{
  if (m_selected.Size()) {
    auto top = m_selected[0];
    if (top == "accessors") {
      return ShowSelected_accessors();
    } else if (top == "meshes") {
      return ShowSelected_meshes();
    } else if (top == "materials") {
      return ShowSelected_materials();
    } else if (top == "images") {
      return ShowSelected_images();
    }
  }

  // default
  return [this]() { ImGui::TextUnformatted(m_selected.Str.c_str()); };
}

// static ImGuiTableFlags flags =
//   ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
//   ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable |
//   ImGuiTableFlags_SortMulti | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders
//   | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_ScrollX |
//   ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;

ShowGui
JsonDockImpl::ShowSelected_accessors()
{
  if (m_selected.Size() == 2) {
    if (auto _i = m_selected.GetInt(1)) {
      auto i = *_i;
      auto accessor = m_scene->m_gltf.Json.at("accessors").at(i);
      // std::string debug = accessor.dump();

      // table

      if (accessor.at("type") == "VEC3" &&
          accessor.at("componentType") == 5126) {
        // float3 table
        if (auto values = m_scene->m_gltf.accessor<DirectX::XMFLOAT3>(i)) {

          return [items = *values]() {
            ImGui::Text("float3[%zu]", items.size());
            if (ImGui::BeginTable("##accessor_values", 4)) {
              ImGui::TableSetupColumn("index");
              ImGui::TableSetupColumn("x");
              ImGui::TableSetupColumn("y");
              ImGui::TableSetupColumn("z");
              ImGui::TableSetupScrollFreeze(0, 1);
              ImGui::TableHeadersRow();
              ImGuiListClipper clipper;
              clipper.Begin(items.size());
              while (clipper.Step()) {
                for (int row_n = clipper.DisplayStart;
                     row_n < clipper.DisplayEnd;
                     row_n++) {
                  auto& value = items[row_n];
                  ImGui::TableNextRow();
                  ImGui::TableSetColumnIndex(0);
                  ImGui::Text("%d", row_n);
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
      }
      int count = accessor.at("count");
      return [count]() { ImGui::Text("%d", count); };
    }
  }
  return []() { ImGui::TextUnformatted("accessors[x] fail"); };
}

ShowGui
JsonDockImpl::ShowSelected_images()
{
  return []() {
    //
    ImGui::TextUnformatted("images");
  };
}

ShowGui
JsonDockImpl::ShowSelected_materials()
{
  return []() {
    //
    ImGui::TextUnformatted("materials");
  };
}

ShowGui
JsonDockImpl::ShowSelected_meshes()
{
  if (m_selected.Size() == 1) {
    return [this]() {
      auto meshes = m_scene->m_gltf.Json.at("meshes");
      if (ImGui::BeginTable("##meshes", 2)) {
        ImGui::TableSetupColumn("index");
        ImGui::TableSetupColumn("name");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();
        for (int i = 0; i < meshes.size(); ++i) {
          auto& mesh = meshes[i];
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("%d", i);
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%s", ((std::string)mesh["name"]).c_str());
        }
        ImGui::EndTable();
      }
    };
  } else if (m_selected.Size() == 2) {
    if (auto _i = m_selected.GetInt(1)) {
      auto i = *_i;
      return
        [this,
         prims = m_scene->m_gltf.Json.at("meshes").at(i).at("primitives")]() {
          if (ImGui::BeginTable("##prims", 4)) {
            ImGui::TableSetupColumn("index");
            ImGui::TableSetupColumn("vertices");
            ImGui::TableSetupColumn("attrs");
            ImGui::TableSetupColumn("indices");
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();
            for (int i = 0; i < prims.size(); ++i) {
              auto& prim = prims[i];
              ImGui::TableNextRow();
              ImGui::TableSetColumnIndex(0);
              ImGui::Text("%d", i);
              ImGui::TableSetColumnIndex(1);
              auto attributes = prim.at("attributes");
              int POSITION = attributes.at("POSITION");
              ImGui::Text("%d",
                          (int)m_scene->m_gltf.Json.at("accessors")
                            .at(POSITION)
                            .at("count"));
              ImGui::TableSetColumnIndex(2);
              std::stringstream ss;
              for (auto kv : attributes.items()) {
                ss << "," << kv.key();
              }
              ImGui::Text("%s", ss.str().c_str());
              ImGui::TableSetColumnIndex(3);
              int indices = prim.at("indices");
              ImGui::Text("%d",
                          (int)m_scene->m_gltf.Json.at("accessors")
                            .at(indices)
                            .at("count"));
            }
            ImGui::EndTable();
          }
        };
    }
  }
  return []() {};
}
