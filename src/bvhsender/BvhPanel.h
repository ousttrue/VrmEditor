#pragma once
#include <DirectXMath.h>
#include <chrono>
#include <functional>
#include <span>
#include <string_view>
#include <vrm/bvh.h>


class BvhPanel
{
  class BvhPanelImpl* impl_ = nullptr;

public:
  BvhPanel();
  ~BvhPanel();
  void SetBvh(const std::shared_ptr<libvrm::bvh::Bvh>& bvh);
  void UpdateGui();
  std::span<const DirectX::XMFLOAT4X4> GetCubes();
};
