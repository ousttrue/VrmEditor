#include "animation.h"
#include "runtime_node.h"

namespace libvrm {

Animation::Animation(std::u8string_view name)
  : m_name(name)
{
}

Time
Animation::Duration() const
{
  float sec = 0;
  for (auto& [k, v] : m_translationMap) {
    sec = std::max(sec, v.MaxSeconds());
  }
  for (auto& [k, v] : m_rotationMap) {
    sec = std::max(sec, v.MaxSeconds());
  }
  for (auto& [k, v] : m_scaleMap) {
    sec = std::max(sec, v.MaxSeconds());
  }
  for (auto& [k, v] : m_weightsMap) {
    sec = std::max(sec, v.MaxSeconds());
  }
  return Time(sec);
}

void
Animation::AddTranslation(uint32_t node_index,
                          std::span<const float> times,
                          std::span<const DirectX::XMFLOAT3> values,
                          std::u8string_view name)
{
  m_translationMap.emplace(node_index,
                           Curve<DirectX::XMFLOAT3>{
                             .Name = { name.begin(), name.end() },
                             .Times = { times.begin(), times.end() },
                             .Values = { values.begin(), values.end() },
                           });
}

void
Animation::AddRotation(uint32_t node_index,
                       std::span<const float> times,
                       std::span<const DirectX::XMFLOAT4> values,
                       std::u8string_view name)
{
  m_rotationMap.emplace(node_index,
                        Curve<DirectX::XMFLOAT4>{
                          .Name = { name.begin(), name.end() },
                          .Times = { times.begin(), times.end() },
                          .Values = { values.begin(), values.end() },
                        });
}

void
Animation::AddScale(uint32_t node_index,
                    std::span<const float> times,
                    std::span<const DirectX::XMFLOAT3> values,
                    std::u8string_view name)
{
  m_scaleMap.emplace(node_index,
                     Curve<DirectX::XMFLOAT3>{
                       .Name = { name.begin(), name.end() },
                       .Times = { times.begin(), times.end() },
                       .Values = { values.begin(), values.end() },
                     });
}

void
Animation::AddWeights(uint32_t node_index,
                      std::span<const float> times,
                      std::span<const float> values,
                      std::u8string_view name)
{
  m_weightsMap.emplace(
    node_index,
    WeightsCurve{
      .Name = { name.begin(), name.end() },
      .Times = { times.begin(), times.end() },
      .Values = { values.begin(), values.end() },
      .WeightsCount = static_cast<uint32_t>(values.size() / times.size()),
    });
}

void
Animation::Update(Time time,
                  std::span<std::shared_ptr<Node>> nodes,
                  RuntimeScene& runtime,
                  bool repeat) const
{
  float seconds = time.count();
  for (auto& [k, v] : m_translationMap) {
    auto node = nodes[k];
    runtime.GetRuntimeNode(node)->Transform.Translation =
      v.GetValue(seconds, repeat);
  }
  for (auto& [k, v] : m_rotationMap) {
    auto node = nodes[k];
    runtime.GetRuntimeNode(node)->Transform.Rotation =
      v.GetValue(seconds, repeat);
  }
  for (auto& [k, v] : m_scaleMap) {
    auto node = nodes[k];
    runtime.GetRuntimeNode(node)->Scale = v.GetValue(seconds, repeat);
  }
  for (auto& [k, v] : m_weightsMap) {
    auto node = nodes[k];
    auto values = v.GetValue(seconds, repeat);
    if (auto meshId = runtime.m_table->m_gltf->Nodes[k].MeshId()) {
      // if (auto instance = runtime->GetDeformedMesh(*meshId)) {
      //   instance->Weights.assign(values.begin(), values.end());
      // }
    }
  }
}

} // namespace
