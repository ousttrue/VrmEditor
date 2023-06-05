#pragma once
#include "../bvh/bvh.h"
#include "srht.h"
#include <chrono>
#include <list>
#include <memory>
#include <mutex>
#include <span>
#include <vector>

#ifdef _WIN32
#define _WIN32_WINNT 0x0601
#endif
#include <asio.hpp>

namespace libvrm {

struct Node;
struct GltfRoot;

namespace srht {
struct Payload;
class UdpSender
{
  asio::ip::udp::socket socket_;
  std::list<std::shared_ptr<Payload>> payloads_;
  std::mutex mutex_;
  std::vector<JointDefinition> m_joints;
  std::vector<DirectX::XMFLOAT4> m_rotations;
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
                    const std::shared_ptr<GltfRoot>& scene);
  void SendFrame(asio::ip::udp::endpoint ep,
                 uint32_t id,
                 const std::shared_ptr<GltfRoot>& scene,
                 bool pack = true);
};

} // namespace
} // namespace
