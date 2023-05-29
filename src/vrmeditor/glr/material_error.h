#pragma once
#include "material_factory.h"
#include "shader_source.h"
#include <gltfjson.h>
#include <grapho/gl3/material.h>

namespace glr {

inline std::expected<MaterialWithUpdater, std::string>
MaterialFactory_Error(const std::shared_ptr<ShaderSourceManager>& shaderSource,
                      const gltfjson::typing::Root& root,
                      const gltfjson::typing::Bin& bin,
                      std::optional<uint32_t>)
{
  auto vs = shaderSource->Get("error.vert");
  shaderSource->RegisterShaderType(vs, ShaderTypes::Error);
  auto fs = shaderSource->Get("error.frag");
  shaderSource->RegisterShaderType(fs, ShaderTypes::Error);
  if (auto shader =
        grapho::gl3::ShaderProgram::Create(vs->Source, fs->Source)) {
    auto material = std::make_shared<grapho::gl3::Material>();
    material->Shader = *shader;
    return MaterialWithUpdater{
      material,
      [s = *shader](
        const grapho::gl3::Material::EnvVars& env, auto& model, auto& shadow) {
        s->Uniform("Projection")->SetMat4(env.projection);
        s->Uniform("View")->SetMat4(env.view);
        s->Uniform("Model")->SetMat4(model.model);
      }
    };
  } else {
    return std::unexpected{ shader.error() };
  }
}

}
