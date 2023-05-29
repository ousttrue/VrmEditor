#pragma once

namespace glr {

inline std::expected<std::shared_ptr<grapho::gl3::Material>, std::string>
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
    return material;
  } else {
    return std::unexpected{ shader.error() };
  }
}

}
