#pragma once
#include "node.h"
#include "timeline.h"
#include <span>
#include <unordered_map>
#include <vector>

namespace libvrm {

enum class AnimationTargets
{
  TRANSLATION,
  ROTATION,
  SCALE,
  WEIGHTS,
};
static const char* AnimationTargetNames[]{
  "translation",
  "rotation",
  "scale",
  "weights",
};

enum class AnimationInterpolationModes
{
  LINEAR,
  STEP,
  CUBICSPLINE,
};
static const char* AnimationInterpolationModeNames[]{
  "LINEAR",
  "STEP",
  "CUBICSPLINE",
};

template<typename T>
struct Curve
{
  std::u8string Name;
  std::vector<float> Times;
  std::vector<T> Values;
  float MaxSeconds() const { return Times.empty() ? 0 : Times.back(); }

  T GetValue(float time, bool repeat) const
  {
    if (!repeat && time > Times.back()) {
      return Values.back();
    }

    while (time > Times.back()) {
      time -= Times.back();
      if (time < 0) {
        time = 0;
      }
    }
    for (int i = 0; i < Times.size(); ++i) {
      if (Times[i] > time) {
        return Values[i];
      }
    }
    return Values.back();
  }
};

struct WeightsCurve
{
  std::string Name;
  std::vector<float> Times;
  std::vector<float> Values;
  const uint32_t WeightsCount;
  float MaxSeconds() const { return Times.empty() ? 0 : Times.back(); }

  std::span<const float> Span(size_t index) const
  {
    auto begin = index * WeightsCount;
    return { Values.data() + begin, Values.data() + begin + WeightsCount };
  }

  std::span<const float> GetValue(float time, bool repeat) const
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
};

struct Animation
{
  std::u8string m_name;
  std::unordered_map<uint32_t, Curve<DirectX::XMFLOAT3>> m_translationMap;
  std::unordered_map<uint32_t, Curve<DirectX::XMFLOAT4>> m_rotationMap;
  std::unordered_map<uint32_t, Curve<DirectX::XMFLOAT3>> m_scaleMap;
  std::unordered_map<uint32_t, WeightsCurve> m_weightsMap;

  libvrm::Time Duration() const
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
    return libvrm::Time(sec);
  }

  Animation(std::u8string_view name)
    : m_name(name)
  {
  }
  Animation(const Animation&) = delete;
  Animation& operator=(const Animation&) = delete;

  void AddTranslation(uint32_t node_index,
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

  void AddRotation(uint32_t node_index,
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

  void AddScale(uint32_t node_index,
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

  void AddWeights(uint32_t node_index,
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
};

}
