#pragma once
#include <vrm/runtime_scene.h>

class Animation
{
  struct AnimationImpl* m_impl;

public:
  Animation();
  ~Animation();
  void SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& root);
  void ShowGui();
};
