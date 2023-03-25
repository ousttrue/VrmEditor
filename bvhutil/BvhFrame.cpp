#include "BvhFrame.h"
#include <numbers>

BvhMat3 BvhMat3::operator*(const BvhMat3 &rhs) {
  // return spanmath::cast<BvhMat3>(spanmath::Mat3(*this) *
  // spanmath::Mat3(rhs));
  return {};
}
// rotate YZ plane
BvhMat3 BvhMat3::RotateXDegrees(float degree) {
  auto rad = static_cast<float>(std::numbers::pi * degree / 180.0f);
  auto s = std::sin(rad);
  auto c = std::cos(rad);
  return {
      1, 0,  0, //
      0, c,  s, //
      0, -s, c  //
  };
}
BvhMat3 BvhMat3::RotateYDegrees(float degree) {
  auto rad = static_cast<float>(std::numbers::pi * degree / 180.0f);
  auto s = std::sin(rad);
  auto c = std::cos(rad);
  return {
      c, 0, -s, //
      0, 1, 0,  //
      s, 0, c,  //
  };
}
BvhMat3 BvhMat3::RotateZDegrees(float degree) {
  auto rad = static_cast<float>(std::numbers::pi * degree / 180.0f);
  auto s = std::sin(rad);
  auto c = std::cos(rad);
  return {
      c,  s, 0, //
      -s, c, 0, //
      0,  0, 1  //
  };
}

std::tuple<BvhOffset, BvhMat3>
BvhFrame::Resolve(const BvhChannels &channels) const {
  BvhOffset pos = {};
  auto rot = BvhMat3{};
  auto index = channels.startIndex;
  for (int ch = 0; ch < channels.size(); ++ch, ++index) {
    switch (channels.values[ch]) {
    case BvhChannelTypes::Xposition:
      pos.x = values[index];
      break;
    case BvhChannelTypes::Yposition:
      pos.y = values[index];
      break;
    case BvhChannelTypes::Zposition:
      pos.z = values[index];
      break;
    case BvhChannelTypes::Xrotation:
      rot = BvhMat3::RotateXDegrees(values[index]) * rot;
      break;
    case BvhChannelTypes::Yrotation:
      rot = BvhMat3::RotateYDegrees(values[index]) * rot;
      break;
    case BvhChannelTypes::Zrotation:
      rot = BvhMat3::RotateZDegrees(values[index]) * rot;
      break;
    case BvhChannelTypes::None:
      break;
    }
  }
  return {pos, rot};
}
