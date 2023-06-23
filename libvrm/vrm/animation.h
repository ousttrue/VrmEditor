#pragma once
#include "node.h"
#include "runtime_scene.h"
#include "timeline.h"
#include <span>
#include <unordered_map>
#include <vector>

namespace libvrm {

inline float
Lerp(float lhs, float rhs, float t)
{
  return lhs + (rhs - lhs) * t;
}

inline DirectX::XMFLOAT3
Lerp(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs, float t)
{
  DirectX::XMFLOAT3 dst;
  DirectX::XMStoreFloat3(&dst,
                         DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&lhs),
                                               DirectX::XMLoadFloat3(&rhs),
                                               t));
  return dst;
}

inline DirectX::XMFLOAT4
Lerp(const DirectX::XMFLOAT4& lhs, const DirectX::XMFLOAT4& rhs, float t)
{
  DirectX::XMFLOAT4 dst;
  DirectX::XMStoreFloat4(&dst,
                         DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&lhs),
                                                    DirectX::XMLoadFloat4(&rhs),
                                                    t));
  return dst;
}

template<typename T>
struct Curve
{
  std::u8string Name;
  std::vector<float> Times;
  std::vector<T> Values;
  gltfjson::AnimationInterpolationModes Interpolation;

  float MaxSeconds() const { return Times.empty() ? 0 : Times.back(); }

  T GetValue(float time, bool repeat) const
  {
    if (!repeat && time > Times.back()) {
      return Values.back();
    }

    while (time > Times.back()) {
      if (Times.back() <= 0) {
        break;
      }
      time -= Times.back();
      if (time < 0) {
        time = 0;
      }
    }

    float lastTime = 0;
    T last = {};
    for (int i = 0; i < Times.size(); ++i) {
      auto current = Times[i];
      auto value = Values[i];
      if (current > time) {
        if (i == 0) {
          return value;
        }
        if (Interpolation == gltfjson::AnimationInterpolationModes::STEP) {
          return last;
        } else {
          // use linear
          // TOOD: cubic
          return Lerp(last, value, (time - lastTime) / (current - lastTime));
        }
        return value;
      }
      last = value;
      lastTime = current;
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
  gltfjson::AnimationInterpolationModes Interpolation;
  float MaxSeconds() const { return Times.empty() ? 0 : Times.back(); }

  mutable std::vector<float> m_lerpBuffer;

  std::span<const float> Span(size_t index) const
  {
    auto begin = index * WeightsCount;
    return { Values.data() + begin, Values.data() + begin + WeightsCount };
  }

  std::span<const float> SpanLerp(size_t lhs, size_t rhs, float t) const
  {
    auto l = Span(lhs);
    auto r = Span(rhs);
    m_lerpBuffer.clear();
    auto ll = l.begin();
    auto rr = r.begin();
    for (; ll != l.end(); ++ll, ++rr) {
      m_lerpBuffer.push_back(Lerp(*ll, *rr, t));
    }
    return m_lerpBuffer;
  }

  std::span<const float> GetValue(float time, bool repeat) const;
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
                      std::u8string_view name,
                      gltfjson::AnimationInterpolationModes interpolation);

  void AddRotation(uint32_t node_index,
                   std::span<const float> times,
                   std::span<const DirectX::XMFLOAT4> values,
                   std::u8string_view name,
                   gltfjson::AnimationInterpolationModes interpolation);

  void AddScale(uint32_t node_index,
                std::span<const float> times,
                std::span<const DirectX::XMFLOAT3> values,
                std::u8string_view name,
                gltfjson::AnimationInterpolationModes interpolation);

  void AddWeights(uint32_t node_index,
                  std::span<const float> times,
                  std::span<const float> values,
                  std::u8string_view name,
                  gltfjson::AnimationInterpolationModes interpolation);

  void Update(Time time, RuntimeScene& runtime, bool repeat = false) const;
};

}
