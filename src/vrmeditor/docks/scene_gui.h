#pragma once
#include <map>
#include <memory>
#include <string>

namespace runtimescene {
struct RuntimeScene;
}

struct SceneNodeSelection;

class SceneGui
{
  struct SceneGuiImpl* m_impl;

public:
  SceneGui(const std::shared_ptr<runtimescene::RuntimeScene>& scene,
           const std::shared_ptr<SceneNodeSelection>& selection,
           float indent);
  ~SceneGui();
  void Show(const char* title, bool* p_open);
};
