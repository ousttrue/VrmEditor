#pragma once
#include <vrm/gltfroot.h>

class HumanoidDock
{
  struct ImHumanoid* m_humanoid;

public:
  HumanoidDock();
  ~HumanoidDock();
  void SetBase(const std::shared_ptr<libvrm::GltfRoot>& base);
  void ShowGui();
};
