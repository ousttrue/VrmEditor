#include "vrm/animation.h"
#include "vrm/mesh.h"

namespace libvrm::gltf {

void
Animation::Update(Time time,
                  std::span<std::shared_ptr<Node>> nodes,
                  bool repeat)
{
  float seconds = time.count();
  for (auto& [k, v] : m_translationMap) {
    auto node = nodes[k];
    node->Transform.Translation = v.GetValue(seconds, repeat);
  }
  for (auto& [k, v] : m_rotationMap) {
    auto node = nodes[k];
    node->Transform.Rotation = v.GetValue(seconds, repeat);
  }
  for (auto& [k, v] : m_scaleMap) {
    auto node = nodes[k];
    node->Scale = v.GetValue(seconds, repeat);
  }
  for (auto& [k, v] : m_weightsMap) {
    auto node = nodes[k];
    auto values = v.GetValue(seconds, repeat);
    node->Instance->weights.assign(values.begin(), values.end());
  }
}

}
