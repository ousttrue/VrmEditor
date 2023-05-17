#include "gltfjson_gui.h"
#include "type_gui.h"
#include <functional>
#include <imgui.h>
#include <list>
#include <string.h>

enum class UITypes
{
  Asset,
  Buffers,
  BufferViews,
  Accessors,
  Images,
  Samplers,
  Textures,
  Materials,
  Meshs,
  Skins,
  Nodes,
  Scenes,
  Animations,
  Extensions,
  TYPE_COUNT,
};

// struct Current
// {
//   TopLevelType Type;
//   uint32_t Index;
// };

struct UI
{
  UITypes Type;
  std::string Label;
  std::list<UI> Children;

  UI* ShowGui(const UI* selected)
  {
    static ImGuiTreeNodeFlags base_flags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
      ImGuiTreeNodeFlags_SpanAvailWidth;

    ImGuiTreeNodeFlags node_flags = base_flags;
    if (this == selected) {
      node_flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (Children.empty()) {
      node_flags |= ImGuiTreeNodeFlags_Leaf;
    }

    bool node_open =
      ImGui::TreeNodeEx((void*)this, node_flags, "%s", Label.c_str());

    UI* clicked = nullptr;

    if (node_open) {
      for (auto& child : Children) {
        if (child.ShowGui(selected)) {
          clicked = &child;
        }
      }
      ImGui::TreePop();
    }

    if (clicked) {
      return clicked;
    } else if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
      return this;
    } else {
      return nullptr;
    }
  }
};

class GltfJsonGuiImpl
{
  gltfjson::format::Root m_gltf;

  std::list<UI> m_list;
  UI* m_selected = nullptr;

public:
  void SetGltf(const gltfjson::format::Root& gltf)
  {
    m_gltf = gltf;

    m_list.clear();
    m_selected = nullptr;

    m_list.push_back({ UITypes::Asset, "asset" });
    m_list.push_back({ UITypes::Buffers, "buffer" });
    m_list.push_back({ UITypes::BufferViews, "bufferViews" });
    m_list.push_back({ UITypes::Accessors, "accessors" });

    {
      int i = 0;
      for (auto& accessor : m_gltf.Accessors) {
        char buf[256];
        snprintf(buf, sizeof(buf), "%d", i++);
        m_list.back().Children.push_back({ UITypes::Accessors, buf });
      }
    }
  }

  void ShowGui()
  {
    { // Left
      ImGui::BeginChild("left pane", ImVec2(200, 0), true);

      UI* node_clicked = nullptr;
      for (auto& ui : m_list) {
        if (auto current = ui.ShowGui(m_selected)) {
          node_clicked = current;
        }
      }
      if (node_clicked) {
        m_selected = node_clicked;
      }

      ImGui::EndChild();
    }

    ImGui::SameLine();

    {
      // Right
      ImGui::BeginGroup();
      ImGui::BeginChild(
        "item view",
        ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1
                                                         // line below us
      ImGui::Text("MyObject: %d", m_selected);
      // ImGui::Separator();
      // if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
      //   if (ImGui::BeginTabItem("Description")) {
      //     ImGui::TextWrapped(
      //       "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed
      //       do " "eiusmod tempor incididunt ut labore et dolore magna
      //       aliqua.
      //       ");
      //     ImGui::EndTabItem();
      //   }
      //   if (ImGui::BeginTabItem("Details")) {
      //     ImGui::Text("ID: 0123456789");
      //     ImGui::EndTabItem();
      //   }
      //   ImGui::EndTabBar();
      // }
      ImGui::EndChild();
      if (ImGui::Button("Revert")) {
      }
      ImGui::SameLine();
      if (ImGui::Button("Save")) {
      }
      ImGui::EndGroup();
    }
  }
};

GltfJsonGui::GltfJsonGui()
  : m_impl(new GltfJsonGuiImpl)
{
}

GltfJsonGui::~GltfJsonGui()
{
  delete m_impl;
}

void
GltfJsonGui::SetGltf(const gltfjson::format::Root& gltf)
{
  m_impl->SetGltf(gltf);
}

void
GltfJsonGui::ShowGui()
{
  m_impl->ShowGui();
}
