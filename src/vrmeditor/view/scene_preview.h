#pragma once
#include <glr/gl3renderer.h>
#include <glr/scene_renderer.h>
#include <vrm/gltfroot.h>
#include <vrm/runtime_scene.h>

using GetSelectedNode = std::function<std::shared_ptr<libvrm::Node>()>;
using GetSelectedRuntimeNode =
  std::function<std::shared_ptr<libvrm::RuntimeNode>()>;
class ScenePreview
{
  struct ScenePreviewImpl* m_impl;

public:
  ScenePreview(const std::shared_ptr<glr::RenderingEnv>& env = {});
  ~ScenePreview();
  void SetGltf(const std::shared_ptr<libvrm::GltfRoot>& root);
  void SetRuntime(const std::shared_ptr<libvrm::RuntimeScene>& runtime);
  void ShowScreenRect(const char* title,
                      const float color[4],
                      float x,
                      float y,
                      float w,
                      float h);
  void ShowFullWindow(const char* title, const float color[4]);
  void ShowGui();
  std::shared_ptr<glr::ViewSettings> Settings();
};
