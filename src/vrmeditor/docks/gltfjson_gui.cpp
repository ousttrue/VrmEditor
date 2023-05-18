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
Push(grapho::imgui::TreeSplitter* splitter, const char* label, T& values)
{
  char buf[256];
  snprintf(buf, sizeof(buf), "%s (%zd)", label, values.Size());
  auto ui = splitter->Push(buf);

  for (uint32_t i = 0; i < values.Size(); ++i) {
    snprintf(
      buf, sizeof(buf), "%02d:%s", i, (const char*)values[i].Name.c_str());
    auto callback = [i, value = &values[i]]() { ::ShowGui(i, *value); };
    splitter->Push(buf, ui, callback);
  }
}

void
GltfJsonGui::SetGltf(gltfjson::format::Root& gltf)
{
  m_splitter->Clear();
  m_splitter->Push("asset");

  // buffer/bufferView/accessor
  Push(m_splitter, "buffers", gltf.Buffers);
  Push(m_splitter, "bufferViews", gltf.BufferViews);
  Push(m_splitter, "accessors", gltf.Accessors);
  // image/sampler/texture/material/mesh
  Push(m_splitter, "images", gltf.Images);
  Push(m_splitter, "samplers", gltf.Samplers);
  Push(m_splitter, "textures", gltf.Textures);
  Push(m_splitter, "materials", gltf.Materials);
  Push(m_splitter, "meshes", gltf.Meshes);
  // skin/node/scene/animation
  Push(m_splitter, "skins", gltf.Skins);
  Push(m_splitter, "nodes", gltf.Nodes);
  Push(m_splitter, "scenes", gltf.Scenes);
  Push(m_splitter, "animations", gltf.Animations);

  // extensions
}

void
GltfJsonGui::ShowGui()
{
  m_splitter->ShowGui();
}
