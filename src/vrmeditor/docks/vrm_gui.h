#pragma once
#include <vrm/runtime_scene.h>

class VrmGui
{
  struct VrmImpl* m_impl;

public:
  VrmGui();
  ~VrmGui();
  void SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime);
  void ShowGui();
};
