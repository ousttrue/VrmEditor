#pragma once
#include <DirectXMath.h>
#include <expected>
#include <functional>
#include <gltfjson.h>
#include <grapho/gl3/material.h>
#include <optional>

namespace glr {

class ShaderSourceManager;

using UpdateShaderFunc =
  std::function<void(const grapho::gl3::Material::EnvVars&,
                     const grapho::gl3::Material::NodeVars&,
                     const DirectX::XMFLOAT4X4& shadow)>;

struct MaterialWithUpdater
{
  std::shared_ptr<grapho::gl3::Material> Material;
  UpdateShaderFunc Updater;

  void Update(const grapho::gl3::Material::EnvVars& env,
              const grapho::gl3::Material::NodeVars& model,
              const DirectX::XMFLOAT4X4& shadow = {})
  {
    if (Updater) {
      Updater(env, model, shadow);
    }
  }

  static std::expected<MaterialWithUpdater, std::string> Create(
    std::expected<std::shared_ptr<grapho::gl3::Material>, std::string> src)
  {
    if (src) {
      return MaterialWithUpdater{ *src };
    } else {
      return std::unexpected{ src.error() };
    }
  }
};

using MaterialFactoryFunc =
  std::function<std::expected<MaterialWithUpdater, std::string>(
    const std::shared_ptr<ShaderSourceManager>& shaderSource,
    const gltfjson::typing::Root& root,
    const gltfjson::typing::Bin& bin,
    std::optional<uint32_t> materialId)>;

}
