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
    .UniformBinds={
      {"Projection",
        [](auto &world, auto &local, auto){return world.ProjectionMatrix();}},
      { "View",[](auto &world, auto &local, auto){return world.ViewMatrix();}},
      {"Shadow",[](auto &world, auto &local, auto){return world.ShadowMatrix();}},
      {"Model",[](auto &world, auto &local, auto){return local.ModelMatrix();}},
    },
  };
  return ptr;
}

}
