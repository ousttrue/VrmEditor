#pragma once
#include <memory>

namespace libvrm {
struct RuntimeScene;
}

class HierarchyGui
{
  struct HierarchyGuiImplAsset* m_asset;
  struct HierarchyGuiImplRuntime* m_runtime;

public:
  HierarchyGui();
  ~HierarchyGui();
  void SetRuntimeScene(const std::shared_ptr<libvrm::RuntimeScene>& scene);
  void ShowGui();
};
