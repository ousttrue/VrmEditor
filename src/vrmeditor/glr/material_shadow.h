#pragma once
#include "material_factory.h"

namespace glr {

inline std::expected<MaterialWithUpdater, std::string>
MaterialFactory_Shadow(const std::shared_ptr<ShaderSourceManager>& shaderSource,
                       const gltfjson::typing::Root& root,
                       const gltfjson::typing::Bin& bin,
                       std::optional<uint32_t>)
{
  auto vs = shaderSource->Get("shadow.vert");
  shaderSource->RegisterShaderType(vs, ShaderTypes::Shadow);
  auto fs = shaderSource->Get("shadow.frag");
  shaderSource->RegisterShaderType(fs, ShaderTypes::Shadow);
  if (auto shader =
        grapho::gl3::ShaderProgram::Create(vs->Source, fs->Source)) {
    auto material = std::make_shared<grapho::gl3::Material>();
    material->Shader = *shader;
    return MaterialWithUpdater{
      material,
      [s = *shader](auto& env, auto& node, auto& shadow) {
        s->Uniform("Projection")->SetMat4(env.projection);
        s->Uniform("View")->SetMat4(env.view);
        s->Uniform("Shadow")->SetMat4(shadow);
        s->Uniform("Model")->SetMat4(node.model);
      },
    };
  } else {
    return std::unexpected{ shader.error() };
  }
}

}
