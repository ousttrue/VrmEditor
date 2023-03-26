#pragma once
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

using OnTime = std::function<void(std::chrono::milliseconds, bool loop)>;

struct Sequence {
  OnTime callback;
  bool loop = true;
  bool playback = true;
  std::optional<std::chrono::milliseconds> startTime;
};

class Timeline {
  std::chrono::milliseconds m_global;
  std::unordered_map<std::string, std::shared_ptr<Sequence>> m_sequences;

public:
  Timeline();
  ~Timeline();

  void setGlobal(std::chrono::milliseconds time) {
    m_global = time;
    for (auto &[k, v] : m_sequences) {
      if (v->playback) {
        if (auto startTime = v->startTime) {
          auto delta = time - *startTime;
          v->callback(delta, v->loop);
        } else {
          // start sequence
          v->startTime = time;
          v->callback({}, v->loop);
        }
      }
    }
  }

  std::chrono::milliseconds global() const { return m_global; }

  std::shared_ptr<Sequence> addSequence(const std::string &name,
                                        std::chrono::milliseconds duration) {
    auto seq = std::make_shared<Sequence>();

    m_sequences.insert({name, seq});

    return seq;
  }
};
