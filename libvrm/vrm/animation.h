#pragma once
#include "node.h"
#include "runtime_scene.h"
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
inline const char* AnimationTargetNames[]{
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
inline const char* AnimationInterpolationModeNames[]{
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

  Animation(std::u8string_view name);
  Animation(const Animation&) = delete;
  Animation& operator=(const Animation&) = delete;
  Time Duration() const;
  void AddTranslation(uint32_t node_index,
                      std::span<const float> times,
                      std::span<const DirectX::XMFLOAT3> values,
                      std::u8string_view name);

  void AddRotation(uint32_t node_index,
                   std::span<const float> times,
                   std::span<const DirectX::XMFLOAT4> values,
                   std::u8string_view name);

  void AddScale(uint32_t node_index,
                std::span<const float> times,
                std::span<const DirectX::XMFLOAT3> values,
                std::u8string_view name);

  void AddWeights(uint32_t node_index,
                  std::span<const float> times,
                  std::span<const float> values,
                  std::u8string_view name);

  void Update(Time time,
              std::span<std::shared_ptr<Node>> nodes,
              RuntimeScene& runtime,
              bool repeat = false) const;
};

}
