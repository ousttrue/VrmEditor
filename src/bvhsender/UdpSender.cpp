#include "UdpSender.h"
#include "Payload.h"
#include <DirectXMath.h>
#include <iostream>
#include <vrm/bvh.h>

UdpSender::UdpSender(asio::io_context& io)
  : socket_(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0))
{
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
UdpSender::SendSkeleton(asio::ip::udp::endpoint ep,
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
UdpSender::SendFrame(asio::ip::udp::endpoint ep,
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
