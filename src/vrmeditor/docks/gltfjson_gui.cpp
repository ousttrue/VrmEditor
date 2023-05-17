#include "gltfjson_gui.h"
#include "type_gui.h"

class GltfJsonGuiImpl
{
  gltfjson::format::Root m_gltf;

public:
  void ShowGui() { ::ShowGui(m_gltf.Asset); }
  void SetGltf(const gltfjson::format::Root& gltf) { m_gltf = gltf; }
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
}

void
GltfJsonGui::ShowGui()
{
  m_impl->ShowGui();
}
