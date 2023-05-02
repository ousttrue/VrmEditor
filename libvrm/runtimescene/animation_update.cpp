#include "vrm/runtimescene/animation_update.h"
#include "vrm/runtimescene/scene.h"
#include "vrm/runtimescene/mesh.h"

namespace runtimescene {

void
AnimationUpdate(const libvrm::gltf::Animation& animation,
                libvrm::Time time,
                std::span<std::shared_ptr<libvrm::gltf::Node>> nodes,
                const std::shared_ptr<RuntimeScene>& runtime,
                bool repeat)
{
  float seconds = time.count();
  for (auto& [k, v] : animation.m_translationMap) {
    auto node = nodes[k];
    node->Transform.Translation = v.GetValue(seconds, repeat);
  }
  for (auto& [k, v] : animation.m_rotationMap) {
    auto node = nodes[k];
    node->Transform.Rotation = v.GetValue(seconds, repeat);
  }
  for (auto& [k, v] : animation.m_scaleMap) {
    auto node = nodes[k];
    node->Scale = v.GetValue(seconds, repeat);
  }
  for (auto& [k, v] : animation.m_weightsMap) {
    auto node = nodes[k];
    auto values = v.GetValue(seconds, repeat);
    auto instance = runtime->GetRuntimeMesh(node->Mesh);
    instance->weights.assign(values.begin(), values.end());
  }
}

}