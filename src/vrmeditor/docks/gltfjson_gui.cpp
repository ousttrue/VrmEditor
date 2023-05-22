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
static void
Push(grapho::imgui::TreeSplitter* splitter,
     const char* label,
     const gltfjson::format::Root& root,
     const gltfjson::format::Bin& bin,
     T& values)
{
  PrintfBuffer buf;
  auto ui = splitter->Push(buf.Printf("%s (%zd)", label, values.Size()));

  for (uint32_t i = 0; i < values.Size(); ++i) {
    auto& value = values[i];
    auto callback = [&root, &bin, &value]() { ::ShowGui(root, bin, value); };

    auto child = splitter->Push(
      buf.Printf("%02d:%s", i, (const char*)values[i].Name.c_str()),
      ui,
      callback);
    for (auto& extension : value.Extensions) {
      splitter->Push(
        buf.Printf("%s", extension.Name.data()), child, [&extension]() {
          ImGui::TextUnformatted((const char*)extension.Value.data());
        });
    }
  }
}

void
GltfJsonGui::SetGltf(gltfjson::format::Root& gltf,
                     const gltfjson::format::Bin& bin)
{
  m_splitter->Clear();
  m_splitter->Push("asset", nullptr, [&gltf]() { ::ShowGui(gltf.Asset); });

  // buffer/bufferView/accessor
  Push(m_splitter, "buffers", gltf, bin, gltf.Buffers);
  Push(m_splitter, "bufferViews", gltf, bin, gltf.BufferViews);
  Push(m_splitter, "accessors", gltf, bin, gltf.Accessors);
  // image/sampler/texture/material/mesh
  Push(m_splitter, "images", gltf, bin, gltf.Images);
  Push(m_splitter, "samplers", gltf, bin, gltf.Samplers);
  Push(m_splitter, "textures", gltf, bin, gltf.Textures);
  Push(m_splitter, "materials", gltf, bin, gltf.Materials);
  Push(m_splitter, "meshes", gltf, bin, gltf.Meshes);
  // skin/node/scene/animation
  Push(m_splitter, "skins", gltf, bin, gltf.Skins);
  Push(m_splitter, "nodes", gltf, bin, gltf.Nodes);
  Push(m_splitter, "scenes", gltf, bin, gltf.Scenes);
  Push(m_splitter, "animations", gltf, bin, gltf.Animations);

  // extensions
  auto extensions = m_splitter->Push(
    "extensions", nullptr, [&gltf]() { ::ShowGui(gltf.Extensions); });
  for (auto& extension : gltf.Extensions) {
    m_splitter->Push(
      (const char*)extension.Name.c_str(), extensions, [&extension]() {
        ImGui::TextUnformatted((const char*)extension.Value.c_str());
      });
  }
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
