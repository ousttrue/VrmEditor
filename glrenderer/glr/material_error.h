#pragma once
#include "material.h"

namespace glr {

inline std::shared_ptr<Material>
MaterialFactory_Error(const gltfjson::Root& root,
                      const gltfjson::Bin& bin,
                      std::optional<uint32_t>)
{
  auto ptr = std::make_shared<Material>();
  *ptr = Material
  {
    .Name = "magenta for Error",
    .VS = {
      .SourceName = "error.vert",
    },
    .FS = {
      .SourceName= "error.frag",
    },
    .UniformVarMap = {
      {"Projection",
        Mat4Var{[](auto &world, auto &local, auto){ return world.ProjectionMatrix();}}},
      {"View",
        Mat4Var{[](auto &world, auto &local, auto){ return world.ViewMatrix();}}},
      {"Model",
        Mat4Var{[](auto &world, auto &local, auto){ return local.ModelMatrix;}}},
    },
  };
  return ptr;
}
}
