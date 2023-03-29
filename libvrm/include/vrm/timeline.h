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
  OnTime Callback;
  bool Loop = true;
  bool IsPlaying = true;
  std::optional<Time> StartTime;
};

struct Timeline {
  Time CurrentTime;
  std::unordered_map<std::string, std::shared_ptr<Track>> Tracks;

  void SetTime(Time time) {
    CurrentTime = time;
    for (auto &[k, v] : Tracks) {
      if (v->IsPlaying) {
        if (auto startTime = v->StartTime) {
          auto delta = time - *startTime;
          v->Callback(delta, v->Loop);
        } else {
          // start track
          v->StartTime = time;
          v->Callback({}, v->Loop);
        }
      }
    }
  }

  std::shared_ptr<Track> AddTrack(const std::string &name, Time duration) {
    auto seq = std::make_shared<Track>();
    Tracks.insert({name, seq});
    return seq;
  }
};
