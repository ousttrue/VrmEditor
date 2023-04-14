#pragma once
#include <asio.hpp>
#include <list>
#include <memory>
#include <mutex>
#include <span>
#include <vrm/bvh.h>
#include <vrm/srht.h>

struct Payload
{
  std::vector<uint8_t> buffer;

  Payload(const Payload&) = delete;
  Payload& operator=(const Payload&) = delete;
  Payload();
  ~Payload();
  void Push(const void* begin, const void* end);
  template<typename T>
  void Push(const T& t)
  {
    Push((const char*)&t, (const char*)&t + sizeof(T));
  }
  void SetSkeleton(std::span<libvrm::srht::JointDefinition> joints);
  void SetFrame(std::chrono::nanoseconds time,
                float x,
                float y,
                float z,
                bool usePack);
};

class UdpSender
{
  asio::ip::udp::socket socket_;
  std::list<std::shared_ptr<Payload>> payloads_;
  std::mutex mutex_;
  std::vector<libvrm::srht::JointDefinition> joints_;

public:
  UdpSender(asio::io_context& io);
  std::shared_ptr<Payload> GetOrCreatePayload();
  void ReleasePayload(const std::shared_ptr<Payload>& payload);
  void SendSkeleton(asio::ip::udp::endpoint ep,
                    const std::shared_ptr<libvrm::bvh::Bvh>& bvh);
  void SendFrame(asio::ip::udp::endpoint ep,
                 const std::shared_ptr<libvrm::bvh::Bvh>& bvh,
                 const libvrm::bvh::Frame& frame,
                 bool pack);
};
