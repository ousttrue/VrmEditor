#pragma once
#include "material_factory.h"

namespace glr {

inline std::shared_ptr<MaterialFactory>
MaterialFactory_Error(const gltfjson::typing::Root& root,
                      const gltfjson::typing::Bin& bin,
                      std::optional<uint32_t>)
{
  auto ptr = std::make_shared<MaterialFactory>();
  *ptr = MaterialFactory
  {
    .Type = ShaderTypes::Error,
    .VS = {
      .SourceName = "error.vert",
    },
    .FS = {
      .SourceName= "error.frag",
    },
    .UniformBinds = {
      {"Projection",
        [](auto &world, auto &local){ return world.ProjectionMatrix();}},
      {"View",
        [](auto &world, auto &local){ return world.ViewMatrix();}},
      {"Model",
        [](auto &world, auto &local){ return local.ModelMatrix();}},
    },
  };
  return ptr;
}
}
