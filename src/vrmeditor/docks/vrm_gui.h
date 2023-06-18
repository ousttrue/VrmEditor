#pragma once
#include "dockspace.h"
#include <vrm/runtime_scene.h>

class VrmGui
{
  std::shared_ptr<libvrm::RuntimeScene> m_scene;

public:

  void SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime)
  {
    m_scene = runtime;
  }

  void ShowExpression();

  // [vrm]
  // meta
  // humanoid
  // expression
  // lookat
  // firstperson
  // spring
  // constraint
  //
  void Show();
};
