#pragma once
#include "gl3renderer.h"
#include "shader_factory.h"
#include "shader_source.h"
#include <grapho/gl3/error_check.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/texture.h>
#include <list>
#include <unordered_map>
#include <vector>

namespace grapho {
namespace gl3 {
class ShaderProgram;
}
}

namespace glr {

using UniformVar = std::variant<IntVar,
                                FloatVar,
                                Vec3Var,
                                Vec4Var,
                                Mat3Var,
                                Mat4Var,
                                RgbVar,
                                RgbaVar>;

struct EnvTextureBind
{
  uint32_t Slot;
  EnvTextureTypes Type;
};
struct EnvCubemapBind
{
  uint32_t Slot;
  EnvCubemapTypes Type;
};

struct Material
{
  std::string Name;
  ShaderFactory VS;
  ShaderFactory FS;
  ShaderFactory GS;
  std::shared_ptr<grapho::gl3::ShaderProgram> Compiled;
  std::list<grapho::gl3::TextureSlot> Textures;
  std::list<EnvCubemapBind> EnvCubemaps;
  std::list<EnvTextureBind> EnvTextures;
  std::unordered_map<std::string, UniformVar> UniformVarMap;
  std::vector<std::optional<UniformVar>> UniformVars;

  std::function<
    void(const WorldInfo& world, const LocalInfo& local, const Gltf& gltf)>
    UpdateState;

  void Activate(const std::shared_ptr<ShaderSourceManager>& shaderSource,
                const WorldInfo& world,
                const LocalInfo& local,
                const Gltf& gltf);
};

using MaterialFactoryFunc =
  std::function<std::shared_ptr<Material>(const gltfjson::Root& root,
                                          const gltfjson::Bin& bin,
                                          std::optional<uint32_t> materialId)>;

struct MaterialFactory
{
  std::string Name;
  MaterialFactoryFunc Factory;

  std::shared_ptr<Material> operator()(const gltfjson::Root& root,
                                       const gltfjson::Bin& bin,
                                       std::optional<uint32_t> materialId)
  {
    return Factory(root, bin, materialId);
  }
};

} // namespace
