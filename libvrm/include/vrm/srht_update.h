#pragma once
#include "scene.h"
#include <DirectXMath.h>
#include <memory>
#include <span>
#include <stdint.h>

namespace srht {
void
UpdateScene(const std::shared_ptr<gltf::Scene>& scene,
            std::vector<DirectX::XMFLOAT4X4>& instances,
            std::span<const uint8_t> data);
}
