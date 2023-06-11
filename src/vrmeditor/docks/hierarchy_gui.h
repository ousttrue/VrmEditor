#pragma once
#include <memory>

namespace libvrm {
struct RuntimeScene;
}

class HierarchyGui
{
  struct HierarchyGuiImpl* m_impl;

public:
  HierarchyGui();
  ~HierarchyGui();
  void SetRuntimeScene(const std::shared_ptr<libvrm::RuntimeScene>& scene);
  void ShowGui();
};
