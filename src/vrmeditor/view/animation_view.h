#pragma once
#include <vrm/runtime_scene.h>

class AnimationView
{
  struct AnimationViewImpl* m_impl;

public:
  AnimationView();
  ~AnimationView();
  void SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& root);
  void ShowGui();
};
