#include "gltfjson_gui.h"
#include "printfbuffer.h"
#include "type_gui.h"
#include <grapho/imgui/widgets.h>

GltfJsonGui::GltfJsonGui()
  : m_splitter(new grapho::imgui::TreeSplitter())
{
}

GltfJsonGui::~GltfJsonGui()
{
  delete m_splitter;
}

template<typename T>
using ShowChild = std::function<void(const gltfjson::Root& root,
                                     const gltfjson::Bin& bin,
                                     T&,
                                     grapho::imgui::TreeSplitter::UI*)>;

template<typename T>
static void
Push(grapho::imgui::TreeSplitter* splitter,
     const char* label,
     const gltfjson::Root& root,
     const gltfjson::Bin& bin,
     gltfjson::JsonArrayBase<T>& values,
     const ShowChild<T>& showChild = {})
{
  PrintfBuffer buf;
  auto ui = splitter->Push(buf.Printf("%s (%zd)", label, values.size()));

  for (uint32_t i = 0; i < values.size(); ++i) {
    auto value = values[i];
    auto callback = [&root, &bin, &value]() { ::ShowGui(root, bin, value); };

    auto child = splitter->Push(
      buf.Printf("%s%02d:%s",
                 "??",
                 i,
                 (const char*)values[i].Name().c_str()),
      ui,
      callback);

    if (showChild) {
      showChild(root, bin, value, child);
    }

    // for (auto& extension : value.Extensions) {
    //   splitter->Push(
    //     buf.Printf(" %s", extension.Name.data()),
    //     child,
    //     [&extension]() {},
    //     [&extension]() { ShowText(extension.Value); });
    // }
    // for (auto& extra : value.Extras) {
    //   splitter->Push(
    //     buf.Printf(" %s", extra.Name.data()),
    //     child,
    //     [&extra]() {},
    //     [&extra]() { ShowText(extra.Value); });
    // }
  }
}

void
GltfJsonGui::SetGltf(gltfjson::Root& gltf,
                     const gltfjson::Bin& bin)
{
  m_splitter->Clear();
  // m_splitter->Push("asset", nullptr, [&gltf]() { ::ShowGui(*gltf.Asset());
  // });

  // buffer/bufferView/accessor
  Push(m_splitter, "󰉋 buffers", gltf, bin, gltf.Buffers);
  Push(m_splitter, "󰉋 bufferViews", gltf, bin, gltf.BufferViews);
  Push(m_splitter, "󰉋 accessors", gltf, bin, gltf.Accessors);
  // image/sampler/texture/material/mesh
  Push(m_splitter, "󰉋 images", gltf, bin, gltf.Images);
  Push(m_splitter, "󰉋 samplers", gltf, bin, gltf.Samplers);
  Push(m_splitter, "󰉋 textures", gltf, bin, gltf.Textures);
  Push(m_splitter, "󰉋 materials", gltf, bin, gltf.Materials);

  ShowChild<gltfjson::Mesh> meshCallback =
    [splitter = m_splitter](const gltfjson::Root& root,
                            const gltfjson::Bin& bin,
                            gltfjson::Mesh& value,
                            grapho::imgui::TreeSplitter::UI* child) {
      PrintfBuffer buf;
      int j = 0;
      for (auto prim : value.Primitives) {
        auto primUi = splitter->Push(
          buf.Printf("primitive [%d]", j++), child, [&root, &bin, &prim]() {
            ::ShowGui(root, bin, prim);
          });

        int k = 0;
        for (auto target : prim.Targets) {
          splitter->Push(
            buf.Printf("target [%d]", k), primUi, [&root, &bin, &target]() {
              ImGui::BeginDisabled(true);
              ::ShowGui(root, bin, target);
              ImGui::EndDisabled();
            });
          ++k;
        }

        // for (auto& extension : prim.Extensions) {
        //   splitter->Push(buf.Printf(" %s", extension.Name.data()),
        //                  primUi,
        //                  [&extension]() {
        //                    ImGui::TextUnformatted(
        //                      (const char*)extension.Value.data());
        //                  });
        // }
        // for (auto& extra : prim.Extras) {
        //   splitter->Push(
        //     buf.Printf(" %s", extra.Name.data()), primUi, [&extra]() {
        //       ImGui::TextUnformatted((const char*)extra.Value.data());
        //     });
        // }
      }
    };
  Push(m_splitter, "󰉋 meshes", gltf, bin, gltf.Meshes, meshCallback);

  // skin/node/scene/animation
  Push(m_splitter, "󰉋 skins", gltf, bin, gltf.Skins);
  Push(m_splitter, "󰉋 nodes", gltf, bin, gltf.Nodes);
  Push(m_splitter, "󰉋 scenes", gltf, bin, gltf.Scenes);
  Push(m_splitter, "󰉋 animations", gltf, bin, gltf.Animations);

  // extensions
  // PrintfBuffer buf;
  // {
  //   auto extensions = m_splitter->Push(
  //     buf.Printf("extensions (%zu)", gltf.Extensions.size()), nullptr, []()
  //     {});
  //
  //   for (auto& extension : gltf.Extensions) {
  //     m_splitter->Push(
  //       buf.Printf(" %s", extension.Name.c_str()),
  //       extensions,
  //       [&extension]() {},
  //       [&extension]() { ShowText(extension.Value); });
  //   }
  // }
  // {
  //   auto extras = m_splitter->Push(
  //     buf.Printf("extras (%zu)", gltf.Extras.size()), nullptr, []() {});
  //
  //   for (auto& extra : gltf.Extras) {
  //     m_splitter->Push(
  //       buf.Printf(" %s", extra.Name.c_str()),
  //       extras,
  //       [&extra]() {},
  //       [&extra]() { ShowText(extra.Value); });
  //   }
  // }
}

void
GltfJsonGui::ShowGui(const char* title, bool* p_open)
{
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
  auto is_open = ImGui::Begin(title, p_open);
  ImGui::PopStyleVar();
  if (is_open) {
    m_splitter->ShowGui();
  }
  ImGui::End();
}

void
GltfJsonGui::ShowGuiSelector(const char* title, bool* p_open)
{
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
  auto is_open = ImGui::Begin(title, p_open);
  ImGui::PopStyleVar();
  if (is_open) {
    m_splitter->ShowSelector();
  }
  ImGui::End();
}

void
GltfJsonGui::ShowGuiProperty(const char* title, bool* p_open)
{
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
  auto is_open = ImGui::Begin(title, p_open);
  ImGui::PopStyleVar();
  if (is_open) {
    m_splitter->ShowSelected();
  }
  ImGui::End();
}

void
GltfJsonGui::ShowGuiText(const char* title, bool* p_open)
{
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
  auto is_open = ImGui::Begin(title, p_open);
  ImGui::PopStyleVar();
  if (is_open) {
    m_splitter->ShowText();
  }
  ImGui::End();
}
