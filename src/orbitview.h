#pragma once
#include <DirectXMath.h>
#include <array>
#include <cmath>

class OrbitView {
  float fovY_ = DirectX::XMConvertToRadians(30.0f);
  int width_ = 1;
  int height_ = 1;
  float nearZ_ = 0.01f;
  float farZ_ = 1000.0f;

  float yaw_ = {};
  float pitch_ = {};
  float shift_[3] = {0, -0.8f, -5};

public:
  OrbitView() {}

  void SetSize(int w, int h) {
    if (w == width_ && h == height_) {
      return;
    }
    width_ = w;
    height_ = h;
  }

  void YawPitch(int dx, int dy) {
    yaw_ += static_cast<float>(dx);
    pitch_ += static_cast<float>(dy);
  }

  void Shift(int dx, int dy) {
    auto factor = std::tan(fovY_ * 0.5f) * 2.0f * shift_[2] / height_;
    shift_[0] -= dx * factor;
    shift_[1] += dy * factor;
  }

  void Dolly(int d) {
    if (d > 0) {
      shift_[2] *= 0.9f;
    } else if (d < 0) {
      shift_[2] *= 1.1f;
    }
  }

  void Update(const float projection[16], const float view[16]) const {
    float aspectRatio = (float)width_ / (float)height_;
    DirectX::XMStoreFloat4x4(
        (DirectX::XMFLOAT4X4 *)projection,
        DirectX::XMMatrixPerspectiveFovRH(fovY_, aspectRatio, nearZ_, farZ_));

    auto yaw = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(yaw_));
    auto pitch =
        DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(pitch_));
    auto shift = DirectX::XMMatrixTranslation(shift_[0], shift_[1], shift_[2]);
    DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4 *)view, yaw * pitch * shift);
  }
};
