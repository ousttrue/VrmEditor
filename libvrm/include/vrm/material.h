#pragma once
#include "texture.h"

namespace libvrm {
namespace gltf {

enum MaterialTypes
{
  Pbr,
  UnLit,
  MToon0,
  MToon1,
};

struct Material
{
  Material(std::string_view name)
    : Name(name)
  {
  }
  std::string Name;
  MaterialTypes Type;
  std::shared_ptr<Texture> ColorTexture;
};

}
}
