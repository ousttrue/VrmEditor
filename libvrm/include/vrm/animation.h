#pragma once
#include "node.h"
#include "scenetypes.h"
#include "timeline.h"
#include <span>
#include <unordered_map>
#include <vector>

template <typename T> struct Curve {
  std::string name;
  std::vector<float> times;
  std::vector<T> values;
  float maxSeconds() const { return times.empty() ? 0 : times.back(); }

  T getValue(float time, bool repeat) const {
    if (!repeat && time > times.back()) {
      return values.back();
    }

    while (time > times.back()) {
      time -= times.back();
      if (time < 0) {
        time = 0;
      }
    }
    for (int i = 0; i < times.size(); ++i) {
      if (times[i] > time) {
        return values[i];
      }
    }
    return values.back();
  }
};

struct WeightsCurve {
  std::string name;
  std::vector<float> times;
  std::vector<float> values;
  // times.size() * weights == values.size()
  const uint32_t weightsCount;
  float maxSeconds() const { return times.empty() ? 0 : times.back(); }

  std::span<const float> span(size_t index) const {
    auto begin = index * weightsCount;
    return {values.data() + begin, values.data() + begin + weightsCount};
  }

  std::span<const float> getValue(float time, bool repeat) const {
    if (!repeat && time > times.back()) {
      return span(times.size() - 1);
    }

    while (time > times.back()) {
      time -= times.back();
      if (time < 0) {
        time = 0;
      }
    }
    for (int i = 0; i < times.size(); ++i) {
      if (times[i] > time) {
        return span(i);
      }
    }

    return span(times.size() - 1);
  }
};

struct Animation {
  std::string m_name;
  std::unordered_map<uint32_t, Curve<DirectX::XMFLOAT3>> m_translationMap;
  std::unordered_map<uint32_t, Curve<DirectX::XMFLOAT4>> m_rotationMap;
  std::unordered_map<uint32_t, Curve<DirectX::XMFLOAT3>> m_scaleMap;
  std::unordered_map<uint32_t, WeightsCurve> m_weightsMap;

  Time duration() const {
    float sec = 0;
    for (auto &[k, v] : m_translationMap) {
      sec = std::max(sec, v.maxSeconds());
    }
    for (auto &[k, v] : m_rotationMap) {
      sec = std::max(sec, v.maxSeconds());
    }
    for (auto &[k, v] : m_scaleMap) {
      sec = std::max(sec, v.maxSeconds());
    }
    for (auto &[k, v] : m_weightsMap) {
      sec = std::max(sec, v.maxSeconds());
    }
    return Time(sec);
  }

  Animation(std::string_view name) : m_name(name) {}
  Animation(const Animation &) = delete;
  Animation &operator=(const Animation &) = delete;

  void addTranslation(uint32_t node_index, std::span<const float> times,
                      std::span<const DirectX::XMFLOAT3> values,
                      std::string_view name) {
    m_translationMap.emplace(node_index,
                             Curve<DirectX::XMFLOAT3>{
                                 .name = {name.begin(), name.end()},
                                 .times = {times.begin(), times.end()},
                                 .values = {values.begin(), values.end()},
                             });
  }

  void addRotation(uint32_t node_index, std::span<const float> times,
                   std::span<const DirectX::XMFLOAT4> values,
                   std::string_view name) {
    m_rotationMap.emplace(node_index,
                          Curve<DirectX::XMFLOAT4>{
                              .name = {name.begin(), name.end()},
                              .times = {times.begin(), times.end()},
                              .values = {values.begin(), values.end()},
                          });
  }

  void addScale(uint32_t node_index, std::span<const float> times,
                std::span<const DirectX::XMFLOAT3> values,
                std::string_view name) {
    m_scaleMap.emplace(node_index, Curve<DirectX::XMFLOAT3>{
                                       .name = {name.begin(), name.end()},
                                       .times = {times.begin(), times.end()},
                                       .values = {values.begin(), values.end()},
                                   });
  }

  void addWeights(uint32_t node_index, std::span<const float> times,
                  std::span<const float> values, std::string_view name) {
    m_weightsMap.emplace(
        node_index,
        WeightsCurve{
            .name = {name.begin(), name.end()},
            .times = {times.begin(), times.end()},
            .values = {values.begin(), values.end()},
            .weightsCount = static_cast<uint32_t>(values.size() / times.size()),
        });
  }

  void update(Time time, std::span<std::shared_ptr<Node>> nodes,
              bool repeat = false) {
    float seconds = time.count();
    for (auto &[k, v] : m_translationMap) {
      auto node = nodes[k];
      node->translation = v.getValue(seconds, repeat);
    }
    for (auto &[k, v] : m_rotationMap) {
      auto node = nodes[k];
      node->rotation = v.getValue(seconds, repeat);
    }
    for (auto &[k, v] : m_scaleMap) {
      auto node = nodes[k];
      node->scale = v.getValue(seconds, repeat);
    }
    for (auto &[k, v] : m_weightsMap) {
      auto node = nodes[k];
      node->m_weights = v.getValue(seconds, repeat);
    }
  }
};
