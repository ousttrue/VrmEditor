#pragma once
#include <vrm/runtime_scene.h>

class HumanoidDock
{
  struct ImHumanoid* m_humanoid;

public:
  HumanoidDock();
  ~HumanoidDock();
  void SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& base);
  void ShowGui();
};
