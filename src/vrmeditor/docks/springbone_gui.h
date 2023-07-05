#pragma once
#include <vrm/runtime_scene.h>

namespace gui {

class SpringBoneGui
{
  struct SpringBoneGuiImpl* m_impl;

public:
  SpringBoneGui();
  ~SpringBoneGui();
  void SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime);
  void ShowGui();
};

} // namespace
