#include "vrm/bvhframe.h"
#include <numbers>

namespace bvh {
Transform
Frame::Resolve(const Channels& channels) const
{
  Transform t{
    .Rotation = { 0, 0, 0, 1 },
    .Translation = channels.init,
  };
  auto index = channels.startIndex;
  for (int ch = 0; ch < channels.size(); ++ch, ++index) {
    switch (channels.types[ch]) {
      case ChannelTypes::Xposition:
        t.Translation.x = values[index];
        break;
      case ChannelTypes::Yposition:
        t.Translation.y = values[index];
        break;
      case ChannelTypes::Zposition:
        t.Translation.z = values[index];
        break;
      case ChannelTypes::Xrotation:
        DirectX::XMStoreFloat4(
          &t.Rotation,
          DirectX::XMQuaternionMultiply(
            DirectX::XMQuaternionRotationAxis(
              { 1, 0, 0 }, DirectX::XMConvertToRadians(values[index])),
            DirectX::XMLoadFloat4(&t.Rotation)));
        break;
      case ChannelTypes::Yrotation:
        DirectX::XMStoreFloat4(
          &t.Rotation,
          DirectX::XMQuaternionMultiply(
            DirectX::XMQuaternionRotationAxis(
              { 0, 1, 0 }, DirectX::XMConvertToRadians(values[index])),
            DirectX::XMLoadFloat4(&t.Rotation)));
        break;
      case ChannelTypes::Zrotation:
        DirectX::XMStoreFloat4(
          &t.Rotation,
          DirectX::XMQuaternionMultiply(
            DirectX::XMQuaternionRotationAxis(
              { 0, 0, 1 }, DirectX::XMConvertToRadians(values[index])),
            DirectX::XMLoadFloat4(&t.Rotation)));
        break;
      case ChannelTypes::None:
        break;
    }
  }
  return t;
}
}
