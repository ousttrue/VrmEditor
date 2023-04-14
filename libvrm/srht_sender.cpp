#include "vrm/srht_sender.h"
#include "vrm/bvh.h"
#include "vrm/node.h"
#include "vrm/scene.h"
#include <DirectXMath.h>
#include <chrono>
#include <iostream>

namespace libvrm {
struct Payload
{
  std::vector<uint8_t> buffer;

  Payload(const Payload&) = delete;
  Payload& operator=(const Payload&) = delete;

  Payload() { std::cout << "Payload::Payload" << std::endl; }

  ~Payload() { std::cout << "Payload::~Payload" << std::endl; }
  void Push(const void* begin, const void* end)
  {
    auto dst = buffer.size();
    auto size = std::distance((const char*)begin, (const char*)end);
    buffer.resize(dst + size);
    std::copy((const char*)begin, (const char*)end, buffer.data() + dst);
  }

  template<typename T>
  void Push(const T& t)
  {
    Push((const char*)&t, (const char*)&t + sizeof(T));
  }

  void SetMagic(std::vector<uint8_t>& buffer, std::string_view magic)
  {
    assert(magic.size() == 8);
    buffer.assign(magic.data(), magic.data() + magic.size());
  }

  void SetSkeleton(std::span<libvrm::srht::JointDefinition> joints)
  {
    SetMagic(buffer, libvrm::srht::SRHT_SKELETON_MAGIC1);

    libvrm::srht::SkeletonHeader header{
      .skeletonId = 0,
      .jointCount = static_cast<uint16_t>(joints.size()),
      .flags = {},
    };
    Push((const char*)&header, (const char*)&header + sizeof(header));
    Push(joints.data(), joints.data() + joints.size());
  }

  void SetFrame(std::chrono::nanoseconds time,
                float x,
                float y,
                float z,
                bool usePack)
  {
    SetMagic(buffer, libvrm::srht::SRHT_FRAME_MAGIC1);

    libvrm::srht::FrameHeader header{
      .time = time.count(),
      .flags = usePack ? libvrm::srht::FrameFlags::USE_QUAT32
                       : libvrm::srht::FrameFlags::NONE,
      .skeletonId = 0,
      .x = x,
      .y = y,
      .z = z,
    };
    Push((const char*)&header, (const char*)&header + sizeof(header));
  }
};

UdpSender::UdpSender(asio::io_context& io)
  : socket_(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0))
{
  m_start = std::chrono::steady_clock::now();
}

std::shared_ptr<Payload>
UdpSender::GetOrCreatePayload()
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (payloads_.empty()) {
    return std::make_shared<Payload>();
  } else {
    auto payload = payloads_.front();
    payloads_.pop_front();
    return payload;
  }
}

void
UdpSender::ReleasePayload(const std::shared_ptr<Payload>& payload)
{
  std::lock_guard<std::mutex> lock(mutex_);
  payloads_.push_back(payload);
}

void
UdpSender::SendBvhSkeleton(asio::ip::udp::endpoint ep,
                           const std::shared_ptr<libvrm::bvh::Bvh>& bvh)
{
  auto payload = GetOrCreatePayload();
  joints_.clear();
  auto scaling = bvh->GuessScaling();
  for (auto joint : bvh->joints) {
    joints_.push_back({
      .parentBoneIndex = joint.parent.value_or(-1),
      .boneType = 0,
      .xFromParent = joint.localOffset.x * scaling,
      .yFromParent = joint.localOffset.y * scaling,
      .zFromParent = joint.localOffset.z * scaling,
    });
  }
  payload->SetSkeleton(joints_);

  socket_.async_send_to(
    asio::buffer(payload->buffer),
    ep,
    [self = this, payload](asio::error_code ec, std::size_t bytes_transferred) {
      self->ReleasePayload(payload);
    });
}

static DirectX::XMFLOAT4X4
ToMat(const DirectX::XMFLOAT3X3& rot,
      const DirectX::XMFLOAT3& pos,
      float scaling)
{

  auto r = DirectX::XMLoadFloat3x3(&rot);
  auto t = DirectX::XMMatrixTranslation(
    pos.x * scaling, pos.y * scaling, pos.z * scaling);
  auto m = r * t;
  DirectX::XMFLOAT4X4 mat;
  DirectX::XMStoreFloat4x4(&mat, m);
  return mat;
}

static DirectX::XMFLOAT4
ToQuat(const DirectX::XMFLOAT3X3& rot)
{

  auto r = DirectX::XMLoadFloat3x3((const DirectX::XMFLOAT3X3*)&rot);
  auto q = DirectX::XMQuaternionRotationMatrix(r);
  DirectX::XMFLOAT4 vec4;
  DirectX::XMStoreFloat4(&vec4, q);
  return vec4;
}

void
UdpSender::SendBvhFrame(asio::ip::udp::endpoint ep,
                        const std::shared_ptr<libvrm::bvh::Bvh>& bvh,
                        const libvrm::bvh::Frame& frame,
                        bool pack)
{
  auto payload = GetOrCreatePayload();

  auto scaling = bvh->GuessScaling();
  for (auto& joint : bvh->joints) {
    auto transform = frame.Resolve(joint.channels);
    // auto rotation = ToQuat(rot);

    if (joint.index == 0) {
      payload->SetFrame(
        std::chrono::duration_cast<std::chrono::nanoseconds>(frame.time),
        transform.Translation.x * scaling,
        transform.Translation.y * scaling,
        transform.Translation.z * scaling,
        pack);
    }
    if (pack) {
      auto packed = libvrm::quat_packer::Pack(transform.Rotation.x,
                                              transform.Rotation.y,
                                              transform.Rotation.z,
                                              transform.Rotation.w);
      payload->Push(packed);
    } else {
      payload->Push(transform.Rotation);
    }
  }

  socket_.async_send_to(
    asio::buffer(payload->buffer),
    ep,
    [self = this, payload](asio::error_code ec, std::size_t bytes_transferred) {
      self->ReleasePayload(payload);
    });
}

static void
PushJoints(std::vector<libvrm::srht::JointDefinition>& joints,
           const std::shared_ptr<gltf::Node>& node,
           const std::function<uint16_t(const std::shared_ptr<gltf::Node>&)>&
             getParentIndex)
{
  joints.push_back({
    .parentBoneIndex = getParentIndex(node),
    .boneType = 0,
    .xFromParent = node->Transform.Translation.x,
    .yFromParent = node->Transform.Translation.y,
    .zFromParent = node->Transform.Translation.z,
  });
  if (auto humanoid = node->Humanoid) {
    joints.back().boneType =
      static_cast<uint16_t>(libvrm::srht::FromVrmBone(humanoid->HumanBone));
  }

  for (auto& child : node->Children) {
    PushJoints(joints, child, getParentIndex);
  }
}

void
UdpSender::SendSkeleton(asio::ip::udp::endpoint ep,
                        uint32_t id,
                        const std::shared_ptr<gltf::Scene>& scene)
{
  auto payload = GetOrCreatePayload();
  joints_.clear();

  auto getParentIndex = [scene](auto& node) -> uint16_t {
    if (auto parent = node->Parent.lock()) {
      for (int i = 0; i < scene->m_nodes.size(); ++i) {
        if (scene->m_nodes[i] == parent) {
          return i;
        }
      }
    }
    return -1;
  };
  PushJoints(joints_, scene->m_roots[0], getParentIndex);
  payload->SetSkeleton(joints_);

  socket_.async_send_to(
    asio::buffer(payload->buffer),
    ep,
    [self = this, payload](asio::error_code ec, std::size_t bytes_transferred) {
      self->ReleasePayload(payload);
    });
}

static void
PushJoints(std::shared_ptr<Payload>& payload,
           const std::shared_ptr<gltf::Node>& node,
           bool pack)
{
  if (pack) {
    auto packed = libvrm::quat_packer::Pack(node->Transform.Rotation.x,
                                            node->Transform.Rotation.y,
                                            node->Transform.Rotation.z,
                                            node->Transform.Rotation.w);
    payload->Push(packed);
  } else {
    payload->Push(node->Transform.Rotation);
  }

  for (auto& child : node->Children) {
    PushJoints(payload, child, pack);
  }
}

void
UdpSender::SendFrame(asio::ip::udp::endpoint ep,
                     uint32_t id,
                     const std::shared_ptr<gltf::Scene>& scene,
                     bool pack)
{
  auto payload = GetOrCreatePayload();

  // root
  auto root = scene->m_roots[0];
  payload->SetFrame(std::chrono::steady_clock::now() - m_start,
                    root->Transform.Translation.x,
                    root->Transform.Translation.y,
                    root->Transform.Translation.z,
                    pack);

  PushJoints(payload, root, pack);

  socket_.async_send_to(
    asio::buffer(payload->buffer),
    ep,
    [self = this, payload](asio::error_code ec, std::size_t bytes_transferred) {
      self->ReleasePayload(payload);
    });
}
}
