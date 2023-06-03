#pragma once
#include "material.h"

namespace glr {

inline std::shared_ptr<Material>
MaterialFactory_Shadow(const gltfjson::typing::Root& root,
                       const gltfjson::typing::Bin& bin,
                       std::optional<uint32_t>)
{
  auto ptr = std::make_shared<Material>();
  *ptr=Material{
    .Type = ShaderTypes::Shadow,
    .VS = {
      "shadow.vert",
    },
    .FS = {
      "shadow.frag",
    },
    .UniformVarMap={
      {"Projection",
        Mat4Var{[](auto &world, auto &local, auto){return world.ProjectionMatrix();}}},
      { "View",Mat4Var{[](auto &world, auto &local, auto){return world.ViewMatrix();}}},
      {"Shadow",Mat4Var{[](auto &world, auto &local, auto){return world.ShadowMatrix();}}},
      {"Model",Mat4Var{[](auto &world, auto &local, auto){return local.ModelMatrix();}}},
    },
  };
  return ptr;
}

}
