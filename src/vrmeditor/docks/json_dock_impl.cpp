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

void
JsonDockImpl::SetSelected(std::string_view selected)
{
  m_selected = selected;
  m_jsonpath.clear();
  for (auto jp : m_selected | std::views::split('.')) {
    {
      m_jsonpath.push_back(std::string_view(jp));
    }
  }
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
  if (jsonpath == m_selected) {
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
    SetSelected(jsonpath);
  }

  return node_open && !is_leaf;
}

void
JsonDockImpl::ShowSelected()
{
  //
  if (m_jsonpath.size() == 2 && m_jsonpath[0] == "accessors") {
    // table
    auto i_view = m_jsonpath[1];
    int i;
    if (std::from_chars(i_view.data(), i_view.data() + i_view.size(), i).ec ==
        std::errc{}) {
      auto accessor = m_scene->m_gltf.Json.at("accessors").at(i);
      std::string debug = accessor.dump();

      static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
        ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable |
        ImGuiTableFlags_SortMulti | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody |
        ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_SizingFixedFit;

      if (accessor.at("type") == "VEC3" &&
          accessor.at("componentType") == 5126) {
        // float3 table
        if (auto values = m_scene->m_gltf.accessor<DirectX::XMFLOAT3>(i)) {
          auto items = *values;
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
              for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd;
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
        }
      } else {
        int count = accessor.at("count");
        ImGui::Text("%d", count);
      }
    }
  } else {
    ImGui::TextUnformatted(m_selected.c_str());
  }
}

// https://github.com/ocornut/imgui/issues/319
bool
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
    ShowSelected();
  }
  ImGui::EndChild();
}
