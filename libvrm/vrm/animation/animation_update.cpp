#include "animation_update.h"
#include "../deformed_mesh.h"
#include "runtime_node.h"
#include "runtime_scene.h"

namespace runtimescene {

void
AnimationUpdate(const Animation& animation,
                libvrm::Time time,
                std::span<std::shared_ptr<libvrm::Node>> nodes,
                const std::shared_ptr<RuntimeScene>& runtime,
                bool repeat)
{
  float seconds = time.count();
  for (auto& [k, v] : animation.m_translationMap) {
    auto node = nodes[k];
    runtime->GetRuntimeNode(node)->Transform.Translation =
      v.GetValue(seconds, repeat);
  }
  for (auto& [k, v] : animation.m_rotationMap) {
    auto node = nodes[k];
    runtime->GetRuntimeNode(node)->Transform.Rotation =
      v.GetValue(seconds, repeat);
  }
  for (auto& [k, v] : animation.m_scaleMap) {
    auto node = nodes[k];
    runtime->GetRuntimeNode(node)->Scale = v.GetValue(seconds, repeat);
  }
  for (auto& [k, v] : animation.m_weightsMap) {
    auto node = nodes[k];
    auto values = v.GetValue(seconds, repeat);
    if (auto mesh = runtime->m_table->m_gltf->Nodes[k].Mesh()) {
      if (auto instance = runtime->GetDeformedMesh(*mesh)) {
        instance->Weights.assign(values.begin(), values.end());
      }
    }
  }
}

}
