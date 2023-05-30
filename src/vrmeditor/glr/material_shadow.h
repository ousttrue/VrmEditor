#pragma once
#include "material_factory.h"

namespace glr {

inline std::shared_ptr<MaterialFactory>
MaterialFactory_Shadow(const gltfjson::typing::Root& root,
                       const gltfjson::typing::Bin& bin,
                       std::optional<uint32_t>)
{
  auto ptr = std::make_shared<MaterialFactory>();
  *ptr=MaterialFactory{
    .Type = ShaderTypes::Shadow,
    .VS = {
      "shadow.vert",
    },
    .FS = {
      "shadow.frag",
    },
    .Updater=[](auto &shader, auto& env, auto& node, auto& shadow) {
        shader->SetUniform("Projection",env.projection);
        shader->SetUniform("View",env.view);
        shader->SetUniform("Shadow",shadow);
        shader->SetUniform("Model",node.model);
      },
  };
  return ptr;
}

}
