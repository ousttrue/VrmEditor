#pragma once
#include "Payload.h"
#include <asio.hpp>
#include <list>
#include <memory>
#include <mutex>
#include <span>
#include <vrm/bvh.h>
#include <vrm/srht.h>

class UdpSender
{
  asio::ip::udp::socket socket_;
  std::list<std::shared_ptr<Payload>> payloads_;
  std::mutex mutex_;
  std::vector<srht::JointDefinition> joints_;

public:
  UdpSender(asio::io_context& io);
  std::shared_ptr<Payload> GetOrCreatePayload();
  void ReleasePayload(const std::shared_ptr<Payload>& payload);
  void SendSkeleton(asio::ip::udp::endpoint ep,
                    const std::shared_ptr<bvh::Bvh>& bvh);
  void SendFrame(asio::ip::udp::endpoint ep,
                 const std::shared_ptr<bvh::Bvh>& bvh,
                 const bvh::Frame& frame,
                 bool pack);
};
