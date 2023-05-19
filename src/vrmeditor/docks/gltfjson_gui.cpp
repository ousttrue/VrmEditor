#include "gltfjson_gui.h"
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
  char buf[256];
  snprintf(buf, sizeof(buf), "%s (%zd)", label, values.Size());
  auto ui = splitter->Push(buf);

  for (uint32_t i = 0; i < values.Size(); ++i) {
    snprintf(
      buf, sizeof(buf), "%02d:%s", i, (const char*)values[i].Name.c_str());
    auto callback = [i, &root, &bin, value = &values[i]]() {
      ::ShowGui(root, bin, *value);
    };
    splitter->Push(buf, ui, callback);
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
