#pragma once
#include "texture.h"

namespace libvrm {
namespace gltf {

struct Material
{
  Material(std::string_view name)
    : Name(name)
  {
  }
  std::string Name;
  std::shared_ptr<Texture> Texture;
};

}
}
