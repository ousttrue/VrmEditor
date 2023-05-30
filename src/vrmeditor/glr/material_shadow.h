#pragma once
#include "material_factory.h"

namespace glr {

inline MaterialFactory
MaterialFactory_Shadow(const gltfjson::typing::Root& root,
                       const gltfjson::typing::Bin& bin,
                       std::optional<uint32_t>)
{
  return MaterialFactory{
    .Type = ShaderTypes::Shadow,
    .VS = {
      "shadow.vert",
    },
    .FS = {
      "shadow.frag",
    },
    .Updater=[](auto &shader, auto& env, auto& node, auto& shadow) {
        shader->Uniform("Projection")->SetMat4(env.projection);
        shader->Uniform("View")->SetMat4(env.view);
        shader->Uniform("Shadow")->SetMat4(shadow);
        shader->Uniform("Model")->SetMat4(node.model);
      },
  };
}

}
