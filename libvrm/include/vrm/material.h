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

struct PbrMetallicRoughness
{
  DirectX::XMFLOAT4 BaseColorFactor = { 1, 1, 1, 1 };
  std::shared_ptr<Texture> BaseColorTexture;
  float MetallicFactor = 1.0f;
  float RoughnessFactor = 1.0f;
  std::shared_ptr<Texture> MetallicRoughnessTexture;
};

struct Material
{
  Material(std::string_view name)
    : Name(name)
  {
  }
  std::string Name;
  MaterialTypes Type = {};
  PbrMetallicRoughness Pbr;
  std::shared_ptr<Texture> NormalTexture;
  float NormalTextureScale = 1.0f;
  std::shared_ptr<Texture> OcclusionTexture;
  float OcclusionTextureStrength = 1.0f;
  std::shared_ptr<Texture> EmissiveTexture;
  DirectX::XMFLOAT3 EmissiveFactor = { 0, 0, 0 };
  BlendMode AlphaMode = {};
  float AlphaCutoff = 0.5f;
  bool DoubleSided = false;
};

}
}
