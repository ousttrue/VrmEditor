#pragma once
#include <memory>

namespace libvrm {
namespace gltf {

class Image;

struct TextureSampler
{};

struct Texture
{
  std::shared_ptr<TextureSampler> Sampler;
  std::shared_ptr<Image> Source;
};

}
}
