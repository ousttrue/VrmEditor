#pragma once
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <string_view>

namespace libvrm {
namespace gltf {

struct Texture;
enum MaterialTypes
{
  Pbr,
  UnLit,
  MToon0,
  MToon1,
};

enum BlendMode
{
  Opaque,
  Mask,
  Blend,
};

struct Material
{
  Material(std::string_view name)
    : Name(name)
  {
  }
  std::string Name;
  MaterialTypes Type = {};
  BlendMode AlphaBlendMode = {};
  float AlphaCutoff = 0.5f;
  DirectX::XMFLOAT4 Color = { 1, 1, 1, 1 };
  std::shared_ptr<Texture> ColorTexture;
};

}
}
