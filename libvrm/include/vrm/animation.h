#pragma once
#include "node.h"
#include "scenetypes.h"
#include <chrono>
#include <span>
#include <unordered_map>
#include <vector>

template <typename T> struct Curve {
  std::string name;
  std::vector<float> times;
  std::vector<T> values;

  T getValue(float time) const {
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

  float maxSeconds() const { return times.empty() ? 0 : times.back(); }
};

struct Animation {
  std::string m_name;
  std::unordered_map<uint32_t, Curve<float3>> m_translationMap;
  std::unordered_map<uint32_t, Curve<quaternion>> m_rotationMap;
  std::unordered_map<uint32_t, Curve<float3>> m_scaleMap;

  float maxSeconds() const {
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
    return sec;
  }

  Animation(std::string_view name) : m_name(name) {}
  Animation(const Animation &) = delete;
  Animation &operator=(const Animation &) = delete;

  void addTranslation(uint32_t node_index, std::span<const float> times,
                      std::span<const float3> values, std::string_view name) {
    m_translationMap.emplace(node_index,
                             Curve<float3>{
                                 .name = {name.begin(), name.end()},
                                 .times = {times.begin(), times.end()},
                                 .values = {values.begin(), values.end()},
                             });
  }
  void addRotation(uint32_t node_index, std::span<const float> times,
                   std::span<const quaternion> values, std::string_view name) {
    m_rotationMap.emplace(node_index,
                          Curve<quaternion>{
                              .name = {name.begin(), name.end()},
                              .times = {times.begin(), times.end()},
                              .values = {values.begin(), values.end()},
                          });
  }
  void addScale(uint32_t node_index, std::span<const float> times,
                std::span<const float3> values, std::string_view name) {
    m_scaleMap.emplace(node_index, Curve<float3>{
                                       .name = {name.begin(), name.end()},
                                       .times = {times.begin(), times.end()},
                                       .values = {values.begin(), values.end()},
                                   });
  }

  void update(std::chrono::milliseconds time,
              std::span<std::shared_ptr<Node>> nodes) {
    float seconds = time.count() * 0.001f;
    for (auto &[k, v] : m_translationMap) {
      auto node = nodes[k];
      node->translation = v.getValue(seconds);
    }
    for (auto &[k, v] : m_rotationMap) {
      auto node = nodes[k];
      node->rotation = v.getValue(seconds);
    }
    for (auto &[k, v] : m_scaleMap) {
      auto node = nodes[k];
      node->scale = v.getValue(seconds);
    }
  }
};