#pragma once
#include <DirectXMath.h>
#include <array>
#include <gltfjson/gltf.h>
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

struct PbrMetallicRoughness
{
  std::array<float, 4> BaseColorFactor = { 1, 1, 1, 1 };
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
  Material(std::u8string_view name)
    : Material(std::string_view{ (const char*)name.data(),
                                 (const char*)name.data() + name.size() })
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
  std::array<float, 3> EmissiveFactor = { 0, 0, 0 };
  gltfjson::format::AlphaModes AlphaMode = {};
  float AlphaCutoff = 0.5f;
  bool DoubleSided = false;
};

}
}
