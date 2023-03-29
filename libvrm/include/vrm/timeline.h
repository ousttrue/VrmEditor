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
  OnTime Callback;
  bool Loop = true;
  bool IsPlaying = true;
  std::optional<Time> StartTime;
};

struct Timeline {
  Time CurrentTime;
  std::vector<std::shared_ptr<Track>> Tracks;

  void SetTime(Time time) {
    CurrentTime = time;
    for (auto &track : Tracks) {
      if (track->IsPlaying) {
        if (auto startTime = track->StartTime) {
          auto delta = time - *startTime;
          track->Callback(delta, track->Loop);
        } else {
          // start track
          track->StartTime = time;
          track->Callback({}, track->Loop);
        }
      }
    }
  }

  void SetDeltaTime(Time time) { SetTime(CurrentTime + time); }

  std::shared_ptr<Track> AddTrack(const std::string &name, Time duration) {
    auto track = std::make_shared<Track>();
    track->Name = name;
    track->Duration = duration;
    Tracks.push_back(track);
    return track;
  }
};
