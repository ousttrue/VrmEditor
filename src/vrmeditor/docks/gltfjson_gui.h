#pragma once
#include <gltfjson.h>

namespace grapho {
namespace imgui {
class TreeSplitter;
}
}

class GltfJsonGui
{
  grapho::imgui::TreeSplitter* m_splitter = nullptr;

public:
  GltfJsonGui();
  ~GltfJsonGui();

  void SetGltf(gltfjson::format::Root& gltf);
  void ShowGui(const char* title, bool* p_open);
};
