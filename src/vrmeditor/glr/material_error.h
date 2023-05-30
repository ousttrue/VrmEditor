#pragma once
#include "material_factory.h"

namespace glr {

inline MaterialFactory
MaterialFactory_Error(const gltfjson::typing::Root& root,
                      const gltfjson::typing::Bin& bin,
                      std::optional<uint32_t>)
{
  return {
    .Type = ShaderTypes::Error,
    .VS = {
      .SourceName = "error.vert",
    },
    .FS = {
      .SourceName= "error.frag",
    },
    .Updater = []( auto &shader, auto& env, auto& model, auto& shadow) {
        shader->Uniform("Projection")->SetMat4(env.projection);
        shader->Uniform("View")->SetMat4(env.view);
        shader->Uniform("Model")->SetMat4(model.model);
      },
  };
}

}
