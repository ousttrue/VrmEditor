#pragma once
#include <functional>
#include <string>
#include <vrm/runtime_scene.h>

using ClearJsonPathFunc = std::function<void(const std::u8string&)>;
class VrmGui
{
  struct VrmImpl* m_impl;

public:
  VrmGui(const ClearJsonPathFunc& clearCache);
  ~VrmGui();
  void SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime);
  void ShowGui();
};
