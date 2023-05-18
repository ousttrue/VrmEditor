#pragma once
#include <gltfjson.h>

class GltfJsonGui
{
  class GltfJsonGuiImpl* m_impl = nullptr;

public:
  GltfJsonGui();
  ~GltfJsonGui();

  void SetGltf(gltfjson::format::Root& gltf);
  void ShowGui();
};
