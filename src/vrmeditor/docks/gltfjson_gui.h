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

  void SetGltf(gltfjson::annotation::Root& gltf,
               const gltfjson::annotation::Bin& bin);

  void ShowGui(const char* title, bool* p_open);
  void ShowGuiSelector(const char* title, bool* p_open);
  void ShowGuiProperty(const char* title, bool* p_open);
  void ShowGuiText(const char* title, bool* p_open);
};
