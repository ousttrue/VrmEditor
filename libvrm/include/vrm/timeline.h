#pragma once
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

using Time = std::chrono::duration<double, std::ratio<1, 1>>;

using OnTime = std::function<void(Time, bool loop)>;

struct Track {
  OnTime callback;
  bool loop = true;
  bool playback = true;
  std::optional<Time> startTime;
};

class Timeline {
  Time m_global;
  std::unordered_map<std::string, std::shared_ptr<Track>> m_track;

public:
  Timeline();
  ~Timeline();

  void clear() { m_track.clear(); }

  void setGlobal(Time time) {
    m_global = time;
    for (auto &[k, v] : m_track) {
      if (v->playback) {
        if (auto startTime = v->startTime) {
          auto delta = time - *startTime;
          v->callback(delta, v->loop);
        } else {
          // start track
          v->startTime = time;
          v->callback({}, v->loop);
        }
      }
    }
  }

  Time global() const { return m_global; }

  std::shared_ptr<Track> addTrack(const std::string &name, Time duration) {
    auto seq = std::make_shared<Track>();

    m_track.insert({name, seq});

    return seq;
  }
};
