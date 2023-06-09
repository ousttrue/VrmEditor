#pragma once
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace asio {
class io_context;
}

namespace libvrm {

using Time = std::chrono::duration<double, std::ratio<1, 1>>;

using OnTime = std::function<bool(Time, bool loop)>;

struct Track
{
  std::string Name;
  Time Duration;
  std::list<OnTime> Callbacks;
  bool Loop = true;
  bool IsPlaying = true;
  std::optional<Time> StartTime;

  void SetTime(Time time)
  {
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
    for (auto it = Callbacks.begin(); it != Callbacks.end();) {
      if ((*it)(delta, Loop)) {
        ++it;
      } else {
        it = Callbacks.erase(it);
      }
    }
  }
};

struct Timeline
{
  bool IsPlaying = false;
  Time CurrentTime;
  std::vector<std::shared_ptr<Track>> Tracks;

  void SetTime(Time time, bool force = false)
  {
    if (force) {

    } else if (!IsPlaying) {
      return;
    }
    CurrentTime = time;
    for (auto& track : Tracks) {
      track->SetTime(time);
    }
  }

  void SetDeltaTime(Time time, bool force = false)
  {
    SetTime(CurrentTime + time, force);
  }

  std::shared_ptr<Track> AddTrack(const std::string& name, Time duration)
  {
    auto track = std::make_shared<Track>();
    track->Name = name;
    track->Duration = duration;
    Tracks.push_back(track);
    return track;
  }
};

using OnTimer = std::function<void(libvrm::Time)>;
class IntervalTimer
{
  class IntervalTimerImpl* m_impl = nullptr;

public:
  IntervalTimer(asio::io_context& io,
                std::chrono::nanoseconds interval,
                const OnTimer& onTimer);
  ~IntervalTimer();
};

} // namespace
