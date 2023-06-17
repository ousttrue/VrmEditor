#include "animation.h"
#include "runtime_node.h"

namespace libvrm {

std::span<const float>
WeightsCurve::GetValue(float time, bool repeat) const
{
  if (!repeat && time > Times.back()) {
    return Span(Times.size() - 1);
  }

  while (time > Times.back()) {
    time -= Times.back();
    if (time < 0) {
      time = 0;
    }
  }
  for (int i = 0; i < Times.size(); ++i) {
    if (Times[i] > time) {
      return Span(i);
    }
  }

  return Span(Times.size() - 1);
}

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
                          std::u8string_view name,
                          gltfjson::AnimationInterpolationModes interpolation)
{
  m_translationMap.emplace(node_index,
                           Curve<DirectX::XMFLOAT3>{
                             .Name = { name.begin(), name.end() },
                             .Times = { times.begin(), times.end() },
                             .Values = { values.begin(), values.end() },
                             .Interpolation = interpolation,
                           });
}

void
Animation::AddRotation(uint32_t node_index,
                       std::span<const float> times,
                       std::span<const DirectX::XMFLOAT4> values,
                       std::u8string_view name,
                       gltfjson::AnimationInterpolationModes interpolation)
{
  m_rotationMap.emplace(node_index,
                        Curve<DirectX::XMFLOAT4>{
                          .Name = { name.begin(), name.end() },
                          .Times = { times.begin(), times.end() },
                          .Values = { values.begin(), values.end() },
                          .Interpolation = interpolation,
                        });
}

void
Animation::AddScale(uint32_t node_index,
                    std::span<const float> times,
                    std::span<const DirectX::XMFLOAT3> values,
                    std::u8string_view name,
                    gltfjson::AnimationInterpolationModes interpolation)
{
  m_scaleMap.emplace(node_index,
                     Curve<DirectX::XMFLOAT3>{
                       .Name = { name.begin(), name.end() },
                       .Times = { times.begin(), times.end() },
                       .Values = { values.begin(), values.end() },
                       .Interpolation = interpolation,
                     });
}

void
Animation::AddWeights(uint32_t node_index,
                      std::span<const float> times,
                      std::span<const float> values,
                      std::u8string_view name,
                      gltfjson::AnimationInterpolationModes interpolation)
{
  m_weightsMap.emplace(
    node_index,
    WeightsCurve{
      .Name = { name.begin(), name.end() },
      .Times = { times.begin(), times.end() },
      .Values = { values.begin(), values.end() },
      .WeightsCount = static_cast<uint32_t>(values.size() / times.size()),
      .Interpolation = interpolation,
    });
}

void
Animation::Update(Time time, RuntimeScene& runtime, bool repeat) const
{
  float seconds = time.count();
  for (auto& [k, v] : m_translationMap) {
    auto node = runtime.m_nodes[k];
    node->Transform.Translation = v.GetValue(seconds, repeat);
  }
  for (auto& [k, v] : m_rotationMap) {
    auto node = runtime.m_nodes[k];
    node->Transform.Rotation = v.GetValue(seconds, repeat);
  }
  for (auto& [k, v] : m_scaleMap) {
    auto node = runtime.m_nodes[k];
    node->Scale = v.GetValue(seconds, repeat);
  }
  for (auto& [k, v] : m_weightsMap) {
    auto values = v.GetValue(seconds, repeat);
    runtime.SetMorphWeights(k, values);
  }
}

} // namespace
