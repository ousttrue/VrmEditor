#include "vrm/timeline.h"
#include <asio.hpp>
#include <iostream>

namespace libvrm {

class IntervalTimerImpl
{
  asio::steady_timer m_timer;
  std::chrono::steady_clock::time_point m_startTime;

public:
  IntervalTimerImpl(asio::io_context& io,
                    std::chrono::nanoseconds interval,
                    const OnTimer& onTimer)
    : m_timer(asio::steady_timer(io))
    , m_startTime(std::chrono::steady_clock::now())
  {
    AsyncWait(interval, onTimer);
  }

  ~IntervalTimerImpl() { m_timer.cancel(); }

  void AsyncWait(std::chrono::nanoseconds interval, const OnTimer& onTimer)
  {
    try {
      m_timer.expires_after(interval);
      m_timer.async_wait(
        [self = this, interval, onTimer, startTime = m_startTime](
          const std::error_code&) {
          auto elapsed = std::chrono::steady_clock::now() - startTime;
          onTimer(elapsed);
          self->AsyncWait(interval, onTimer);
        });
    } catch (std::exception const& e) {
      std::cout << "AsyncWait catch: " << e.what() << std::endl;
    }
  }
};

IntervalTimer::IntervalTimer(asio::io_context& io,
                             std::chrono::nanoseconds interval,
                             const OnTimer& onTimer)
  : m_impl(new IntervalTimerImpl(io, interval, onTimer))
{
}

IntervalTimer::~IntervalTimer()
{
  delete m_impl;
}

}
