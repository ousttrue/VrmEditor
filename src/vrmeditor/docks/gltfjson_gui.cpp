#include "gltfjson_gui.h"
#include "type_gui.h"
#include <functional>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <list>
#include <optional>
#include <string.h>

class GltfJsonGuiImpl
{
  struct UI
  {
    std::string Label;
    std::function<void()> Show;
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
  };

  std::list<UI> m_list;
  UI* m_selected = nullptr;
  float m_splitter = 200.0f;

public:
  GltfJsonGuiImpl() {}

  void SetGltf(gltfjson::format::Root& gltf)
  {
    m_list.clear();
    m_selected = nullptr;
    m_list.push_back({ "asset" });

    // buffer/bufferView/accessor
    Push("buffers", gltf.Buffers);
    Push("bufferViews", gltf.BufferViews);
    Push("accessors", gltf.Accessors);
    // image/sampler/texture/material/mesh
    Push("images", gltf.Images);
    Push("samplers", gltf.Samplers);
    Push("textures", gltf.Textures);
    Push("materials", gltf.Materials);
    Push("meshes", gltf.Meshes);
    // skin/node/scene/animation
    Push("skins", gltf.Skins);
    Push("nodes", gltf.Nodes);
    Push("scenes", gltf.Scenes);
    Push("animations", gltf.Animations);

    // extensions
  }

  template<typename T>
  void Push(const char* label, T& values)
  {
    char buf[256];
    snprintf(buf, sizeof(buf), "%s (%zd)", label, values.Size());
    m_list.push_back({ .Label = buf });

    for (uint32_t i = 0; i < values.Size(); ++i) {
      snprintf(
        buf, sizeof(buf), "%02d:%s", i, (const char*)values[i].Name.c_str());
      auto callback = [i, value = &values[i]]() { ::ShowGui(i, *value); };
      m_list.back().Children.push_back({
        .Label = buf,
        .Show = callback,
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
    if (m_selected && m_selected->Show) {
      m_selected->Show();
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
GltfJsonGui::SetGltf(gltfjson::format::Root& gltf)
{
  m_impl->SetGltf(gltf);
}

void
GltfJsonGui::ShowGui()
{
  m_impl->ShowGui();
}
