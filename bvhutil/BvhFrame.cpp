#include "BvhFrame.h"
#include <numbers>

BvhMat3 BvhMat3::operator*(const BvhMat3 &rhs) {
  return {
      _0 * rhs._0 + _1 * rhs._3 + _2 * rhs._6,
      _0 * rhs._1 + _1 * rhs._4 + _2 * rhs._7,
      _0 * rhs._2 + _1 * rhs._5 + _2 * rhs._8,
      _3 * rhs._0 + _4 * rhs._3 + _5 * rhs._6,
      _3 * rhs._1 + _4 * rhs._4 + _5 * rhs._7,
      _3 * rhs._2 + _4 * rhs._5 + _5 * rhs._8,
      _6 * rhs._0 + _7 * rhs._3 + _8 * rhs._6,
      _6 * rhs._1 + _7 * rhs._4 + _8 * rhs._7,
      _6 * rhs._2 + _7 * rhs._5 + _8 * rhs._8,
  };
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

BvhTransform BvhFrame::Resolve(const BvhChannels &channels) const {
  BvhTransform t{
      .Rotation = BvhMat3{},
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
      t.Rotation = BvhMat3::RotateXDegrees(values[index]) * t.Rotation;
      break;
    case BvhChannelTypes::Yrotation:
      t.Rotation = BvhMat3::RotateYDegrees(values[index]) * t.Rotation;
      break;
    case BvhChannelTypes::Zrotation:
      t.Rotation = BvhMat3::RotateZDegrees(values[index]) * t.Rotation;
      break;
    case BvhChannelTypes::None:
      break;
    }
  }
  return t;
}
