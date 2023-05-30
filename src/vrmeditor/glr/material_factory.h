#pragma once
#include "shader_source.h"
#include <DirectXMath.h>
#include <expected>
#include <functional>
#include <gltfjson.h>
#include <grapho/gl3/material.h>
#include <optional>
#include <variant>

namespace glr {

using UpdateShaderFunc =
  std::function<void(const std::shared_ptr<grapho::gl3::ShaderProgram>& shader,
                     const grapho::gl3::Material::EnvVars&,
                     const grapho::gl3::Material::DrawVars&,
                     const DirectX::XMFLOAT4X4& shadow)>;

struct ShaderDefinition
{
  std::u8string Name;
  std::variant<std::monostate, bool, int, float> Value;
};

struct ShaderFactory
{
  std::string SourceName;
  std::u8string Version;
  std::vector<ShaderDefinition> Macros;
  std::u8string MergedSource;
};

struct MaterialFactory
{
  ShaderTypes Type;
  ShaderFactory VS;
  ShaderFactory FS;
  std::expected<std::shared_ptr<grapho::gl3::ShaderProgram>, std::string>
    Compiled;
  std::shared_ptr<grapho::gl3::Material> Material;
  UpdateShaderFunc Updater;

  void Activate(const std::shared_ptr<ShaderSourceManager>& shaderSource,
                const grapho::gl3::Material::EnvVars& env,
                const grapho::gl3::Material::DrawVars& model,
                const DirectX::XMFLOAT4X4& shadow = {})
  {
    if (!Material) {
      Material = std::make_shared<grapho::gl3::Material>();
      auto vs = shaderSource->Get(VS.SourceName);
      shaderSource->RegisterShaderType(vs, Type);
      auto fs = shaderSource->Get(FS.SourceName);
      shaderSource->RegisterShaderType(fs, Type);
      Compiled = grapho::gl3::ShaderProgram::Create(vs->Source, fs->Source);
      if (Compiled) {
        Material->Shader = *Compiled;
      }
    }
    if (Material && Material->Shader) {
      Material->Activate();
      if (Updater) {
        Updater(Material->Shader, env, model, shadow);
      }
    }
  }
};

using MaterialFactoryFunc =
  std::function<MaterialFactory(const gltfjson::typing::Root& root,
                                const gltfjson::typing::Bin& bin,
                                std::optional<uint32_t> materialId)>;
}
