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
     T& values)
{
  char buf[256];
  snprintf(buf, sizeof(buf), "%s (%zd)", label, values.Size());
  auto ui = splitter->Push(buf);

  for (uint32_t i = 0; i < values.Size(); ++i) {
    snprintf(
      buf, sizeof(buf), "%02d:%s", i, (const char*)values[i].Name.c_str());
    auto callback = [i, &root, value = &values[i]]() {
      ::ShowGui(root, *value);
    };
    splitter->Push(buf, ui, callback);
  }
}

void
GltfJsonGui::SetGltf(gltfjson::format::Root& gltf)
{
  m_splitter->Clear();
  m_splitter->Push("asset");

  // buffer/bufferView/accessor
  Push(m_splitter, "buffers", gltf, gltf.Buffers);
  Push(m_splitter, "bufferViews", gltf, gltf.BufferViews);
  Push(m_splitter, "accessors", gltf, gltf.Accessors);
  // image/sampler/texture/material/mesh
  Push(m_splitter, "images", gltf, gltf.Images);
  Push(m_splitter, "samplers", gltf, gltf.Samplers);
  Push(m_splitter, "textures", gltf, gltf.Textures);
  Push(m_splitter, "materials", gltf, gltf.Materials);
  Push(m_splitter, "meshes", gltf, gltf.Meshes);
  // skin/node/scene/animation
  Push(m_splitter, "skins", gltf, gltf.Skins);
  Push(m_splitter, "nodes", gltf, gltf.Nodes);
  Push(m_splitter, "scenes", gltf, gltf.Scenes);
  Push(m_splitter, "animations", gltf, gltf.Animations);

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
