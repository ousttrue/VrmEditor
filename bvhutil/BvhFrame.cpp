#include "BvhFrame.h"
#include <numbers>

BvhTransform BvhFrame::Resolve(const BvhChannels &channels) const {
  BvhTransform t{
      .Rotation = {0, 0, 0, 1},
      .Translation = channels.init,
  };
  auto index = channels.startIndex;
  for (int ch = 0; ch < channels.size(); ++ch, ++index) {
    switch (channels.types[ch]) {
    case BvhChannelTypes::Xposition:
      t.Translation.x = values[index];
      break;
    case BvhChannelTypes::Yposition:
      t.Translation.y = values[index];
      break;
    case BvhChannelTypes::Zposition:
      t.Translation.z = values[index];
      break;
    case BvhChannelTypes::Xrotation:
      DirectX::XMStoreFloat4(
          &t.Rotation,
          DirectX::XMQuaternionMultiply(
              DirectX::XMQuaternionRotationAxis(
                  {1, 0, 0}, DirectX::XMConvertToRadians(values[index])),
              DirectX::XMLoadFloat4(&t.Rotation)));
      break;
    case BvhChannelTypes::Yrotation:
      DirectX::XMStoreFloat4(
          &t.Rotation,
          DirectX::XMQuaternionMultiply(
              DirectX::XMQuaternionRotationAxis(
                  {0, 1, 0}, DirectX::XMConvertToRadians(values[index])),
              DirectX::XMLoadFloat4(&t.Rotation)));
      break;
    case BvhChannelTypes::Zrotation:
      DirectX::XMStoreFloat4(
          &t.Rotation,
          DirectX::XMQuaternionMultiply(
              DirectX::XMQuaternionRotationAxis(
                  {0, 0, 1}, DirectX::XMConvertToRadians(values[index])),
              DirectX::XMLoadFloat4(&t.Rotation)));
      break;
    case BvhChannelTypes::None:
      break;
    }
  }
  return t;
}
