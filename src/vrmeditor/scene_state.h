#pragma once
#include <vrm/runtime_scene.h>

struct SceneState
{
  std::shared_ptr<libvrm::RuntimeScene> m_runtime;
  std::optional<libvrm::Time> m_lastTime;
  using SetSceneFunc =
    std::function<void(const std::shared_ptr<libvrm::RuntimeScene>&)>;
  std::list<SetSceneFunc> m_setCallbacks;

  static SceneState& GetInstance()
  {
    static SceneState s_state;
    return s_state;
  }

  void SetGltf(const std::shared_ptr<libvrm::GltfRoot>& gltf);

  bool LoadModel(const std::filesystem::path& path);

  bool LoadGltfString(const std::string& json);

  void Update(libvrm::Time time);

  bool WriteScene(const std::filesystem::path& path);

  std::string CopyVrmPoseText();
};
