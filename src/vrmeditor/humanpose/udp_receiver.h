#pragma once
#include <functional>
#include <span>
#include <stdint.h>

using OnRecv = std::function<void(std::span<const uint8_t>)>;

class UdpReceiver
{
  struct UdpReceiverImpl* m_impl = nullptr;

public:
  UdpReceiver();
  ~UdpReceiver();
  void Update();
  void Start(uint16_t port, const OnRecv& callback);
  void Stop(uint16_t port);
};
