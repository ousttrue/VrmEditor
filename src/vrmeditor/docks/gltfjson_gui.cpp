#include "gltfjson_gui.h"
#include "type_gui.h"
#include <functional>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <list>
#include <optional>
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
  std::optional<uint32_t> Index;
  std::list<UI> Children;

  UI* ShowSelector(const UI* selected)
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
        if (child.ShowSelector(selected)) {
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

  void ShowSelected(gltfjson::format::Root& gltf)
  {
    switch (Type) {
      case UITypes::Asset:
        ::ShowGui(gltf.Asset);
        break;

        // buffer/bufferView/accessor
      case UITypes::Buffers:
        if (Index) {
          ::ShowGui(*Index, gltf.Buffers[*Index]);
        } else {
        }
        break;

      case UITypes::BufferViews:
        if (Index) {
          ::ShowGui(*Index, gltf.BufferViews[*Index]);
        } else {
        }
        break;

      case UITypes::Accessors:
        if (Index) {
          ::ShowGui(*Index, gltf.Accessors[*Index]);
        } else {
        }
        break;
        // image/sampler/texture/material/mesh
      case UITypes::Images:
        if (Index) {
          ::ShowGui(*Index, gltf.Images[*Index]);
        } else {
        }
        break;
      case UITypes::Samplers:
        if (Index) {
          ::ShowGui(*Index, gltf.Samplers[*Index]);
        } else {
        }
        break;
      case UITypes::Textures:
        if (Index) {
          ::ShowGui(*Index, gltf.Textures[*Index]);
        } else {
        }
        break;
      case UITypes::Materials:
        if (Index) {
          ::ShowGui(*Index, gltf.Materials[*Index]);
        } else {
        }
        break;
      case UITypes::Meshes:
        if (Index) {
          ::ShowGui(*Index, gltf.Meshes[*Index]);
        } else {
        }
        break;
        // skin/node/scene/animation
      case UITypes::Skins:
        if (Index) {
          ::ShowGui(*Index, gltf.Skins[*Index]);
        } else {
        }
        break;
      case UITypes::Nodes:
        if (Index) {
          ::ShowGui(*Index, gltf.Nodes[*Index]);
        } else {
        }
        break;
      case UITypes::Scenes:
        if (Index) {
          ::ShowGui(*Index, gltf.Scenes[*Index]);
        } else {
        }
        break;
      case UITypes::Animations:
        if (Index) {
          ::ShowGui(*Index, gltf.Animations[*Index]);
        } else {
        }
        break;
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

    for (uint32_t i = 0; i < values.Size(); ++i) {
      snprintf(
        buf, sizeof(buf), "%02d:%s", i, (const char*)values[i].Name.c_str());
      m_list.back().Children.push_back({
        .Type = type,
        .Label = buf,
        .Index = i,
      });
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
      if (auto current = ui.ShowSelector(m_selected)) {
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

    // Leave room for 1 line below us
    ImGui::BeginChild("item view",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
    if (m_selected) {
      m_selected->ShowSelected(m_gltf);
    }
    ImGui::EndChild();
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
