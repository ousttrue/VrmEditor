#pragma once
#include "material_factory.h"

namespace glr {

inline std::shared_ptr<MaterialFactory>
MaterialFactory_Error(const gltfjson::typing::Root& root,
                      const gltfjson::typing::Bin& bin,
                      std::optional<uint32_t>)
{
  auto ptr = std::make_shared<MaterialFactory>();
  *ptr=MaterialFactory{
    .Type = ShaderTypes::Error,
    .VS = {
      .SourceName = "error.vert",
    },
    .FS = {
      .SourceName= "error.frag",
    },
    .Updater = []( auto &shader, auto& env, auto& model, auto& shadow) {
        shader->SetUniform("Projection",env.projection);
        shader->SetUniform("View",env.view);
        shader->SetUniform("Model",model.model);
      },
  };
  return ptr;
}

}
