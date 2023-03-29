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
  std::string Name;
  Time Duration;
  std::list<OnTime> Callbacks;
  bool Loop = true;
  bool IsPlaying = true;
  std::optional<Time> StartTime;

  void SetTime(Time time) {
    if (!IsPlaying) {
      return;
    }
    Time delta;
    if (auto startTime = StartTime) {
      // already started
      delta = time - *startTime;
    } else {
      // start track
      // delta is zero
      StartTime = time;
    }
    for (auto &callback : Callbacks) {
      callback(delta, Loop);
    }
  }
};

struct Timeline {
  bool IsPlaying = false;
  Time CurrentTime;
  std::vector<std::shared_ptr<Track>> Tracks;

  void SetTime(Time time, bool force = false) {
    if (force) {

    } else if (!IsPlaying) {
      return;
    }
    CurrentTime = time;
    for (auto &track : Tracks) {
      track->SetTime(time);
    }
  }

  void SetDeltaTime(Time time, bool force = false) {
    SetTime(CurrentTime + time, force);
  }

  std::shared_ptr<Track> AddTrack(const std::string &name, Time duration) {
    auto track = std::make_shared<Track>();
    track->Name = name;
    track->Duration = duration;
    Tracks.push_back(track);
    return track;
  }
};
