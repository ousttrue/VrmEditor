#pragma once
#include "material.h"

namespace glr {

inline std::shared_ptr<Material>
MaterialFactory_Shadow(const gltfjson::Root& root,
                       const gltfjson::Bin& bin,
                       std::optional<uint32_t>)
{
  auto ptr = std::make_shared<Material>();
  *ptr=Material{
    .Name = "ShadowMatrix shader",
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
      {"Model",Mat4Var{[](auto &world, auto &local, auto){return local.ModelMatrix;}}},
    },
  };
  return ptr;
}

inline std::shared_ptr<Material>
MaterialFactory_Wireframe(const gltfjson::Root& root,
                          const gltfjson::Bin& bin,
                          std::optional<uint32_t>)
{
  auto ptr = std::make_shared<Material>();
  *ptr=Material{
    .Name = "Wireframe shader",
    .VS = {
      "wireframe.vert",
    },
    .FS = {
      "wireframe.frag",
    },
    .GS = {
      "wireframe.geom",
    },
    .UniformVarMap={
      {"Projection",
        Mat4Var{[](auto &world, auto &local, auto){return world.ProjectionMatrix();}}},
      { "View",Mat4Var{[](auto &world, auto &local, auto){return world.ViewMatrix();}}},
      {"Model",Mat4Var{[](auto &world, auto &local, auto){return local.ModelMatrix;}}},
    },
  };
  return ptr;
}

}
