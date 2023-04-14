#pragma once
#include <asio.hpp>
#include <chrono>
#include <list>
#include <memory>
#include <mutex>
#include <span>
#include <vector>
#include <vrm/bvh.h>
#include <vrm/srht.h>

namespace libvrm {
namespace gltf {
struct Node;
struct Scene;
}

struct Payload;
class UdpSender
{
  asio::ip::udp::socket socket_;
  std::list<std::shared_ptr<Payload>> payloads_;
  std::mutex mutex_;
  std::vector<srht::JointDefinition> joints_;
  std::chrono::steady_clock::time_point m_start;

public:
  UdpSender(asio::io_context& io);
  std::shared_ptr<Payload> GetOrCreatePayload();
  void ReleasePayload(const std::shared_ptr<Payload>& payload);

  void SendBvhSkeleton(asio::ip::udp::endpoint ep,
                       const std::shared_ptr<bvh::Bvh>& bvh);
  void SendBvhFrame(asio::ip::udp::endpoint ep,
                    const std::shared_ptr<bvh::Bvh>& bvh,
                    const bvh::Frame& frame,
                    bool pack = true);

  void SendSkeleton(asio::ip::udp::endpoint ep,
                    uint32_t id,
                    const std::shared_ptr<gltf::Scene>& scene);
  void SendFrame(asio::ip::udp::endpoint ep,
                 uint32_t id,
                 const std::shared_ptr<gltf::Scene>& scene,
                 bool pack = true);
};
}
