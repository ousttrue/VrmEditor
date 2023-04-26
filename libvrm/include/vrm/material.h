#pragma once
#include <memory>
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
  BlendMode AlphaBlend = {};
  std::shared_ptr<Texture> ColorTexture;
};

}
}
