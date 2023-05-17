#include "gltfjson_gui.h"
#include "type_gui.h"
#include <functional>
#include <grapho/imgui/widgets.h>
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
  Meshes,
  Skins,
  Nodes,
  Scenes,
  Animations,
  Extensions,
  TYPE_COUNT,
};

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
  float m_splitter = 200.0f;

public:
  GltfJsonGuiImpl() {}

  void SetGltf(const gltfjson::format::Root& gltf)
  {
    m_gltf = gltf;

    m_list.clear();
    m_selected = nullptr;
    m_list.push_back({ UITypes::Asset, "asset" });

    // buffer/bufferView/accessor
    Push(UITypes::Buffers, "buffers", m_gltf.Buffers);
    Push(UITypes::BufferViews, "bufferViews", m_gltf.BufferViews);
    Push(UITypes::Accessors, "accessors", m_gltf.Accessors);
    // image/sampler/texture/material/mesh
    Push(UITypes::Images, "images", m_gltf.Images);
    Push(UITypes::Samplers, "samplers", m_gltf.Samplers);
    Push(UITypes::Textures, "textures", m_gltf.Textures);
    Push(UITypes::Materials, "materials", m_gltf.Materials);
    Push(UITypes::Meshes, "meshes", m_gltf.Meshes);
    // skin/node/scene/animation
    Push(UITypes::Skins, "skins", m_gltf.Skins);
    Push(UITypes::Nodes, "nodes", m_gltf.Nodes);
    Push(UITypes::Scenes, "scenes", m_gltf.Scenes);
    Push(UITypes::Animations, "animations", m_gltf.Animations);

    // extensions
  }

  template<typename T>
  void Push(UITypes type, const char* label, const T& values)
  {
    char buf[256];
    snprintf(buf, sizeof(buf), "%s (%zd)", label, values.Size());
    m_list.push_back({ type, buf });

    for (auto i = 0; i < values.Size(); ++i) {
      snprintf(
        buf, sizeof(buf), "%02d:%s", i, (const char*)values[i].Name.c_str());
      m_list.back().Children.push_back({ type, buf });
    }
  }

  void ShowGui()
  {
    // auto size = ImGui::GetCurrentWindow()->Size;
    auto size = ImGui::GetContentRegionAvail();
    float s = size.x - m_splitter - 5;
    // ImGui::Text("%f, %f: %f; %f", size.x, size.y, f, s);
    grapho::imgui::Splitter(true, 5, &m_splitter, &s, 8, 8);

    ShowGuiLeft(m_splitter);
    ShowGuiRight();
  }

  void ShowGuiLeft(float w)
  {
    ImGui::BeginChild("left pane", ImVec2(w, 0), true);

    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0);
    UI* node_clicked = nullptr;
    for (auto& ui : m_list) {
      if (auto current = ui.ShowGui(m_selected)) {
        node_clicked = current;
      }
    }
    if (node_clicked) {
      m_selected = node_clicked;
    }
    ImGui::PopStyleVar();

    ImGui::EndChild();
  }

  void ShowGuiRight()
  {
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::BeginChild(
      "item view",
      ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1
                                                       // line below us
    ImGui::Text("MyObject: %p", m_selected);
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
};

GltfJsonGui::GltfJsonGui()
  : m_impl(new GltfJsonGuiImpl())
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
