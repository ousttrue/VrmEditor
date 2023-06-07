#pragma once
#include "material.h"
#include "material_three_vrm0.h"
#include "material_three_vrm1.h"
#include <gltfjson.h>

namespace glr {

struct Material;
std::shared_ptr<Material>
MaterialFactory_MToon(const gltfjson::Root& root,
                      const gltfjson::Bin& bin,
                      std::optional<uint32_t> materialId)
{
  if (materialId) {
    auto material = gltfjson::Material(root.Materials[*materialId]);
    gltfjson::tree::NodePtr mtoon1;
    if (auto extensions = material.Extensions()) {
      mtoon1 = extensions->Get(u8"VRMC_materials_mtoon");
    }

    gltfjson::tree::NodePtr mtoon0;
    if (auto root_extensins = root.Extensions()) {
      if (auto VRM = root_extensins->Get(u8"VRM")) {
        if (auto props = VRM->Get(u8"materialProperties")) {
          if (auto array = props->Array()) {
            if (*materialId < array->size()) {
              auto mtoonMaterial = (*array)[*materialId];
              if (auto shader = mtoonMaterial->Get(u8"shader")) {
                if (shader->U8String() == u8"VRM/MToon") {
                  mtoon0 = mtoonMaterial;
                }
              }
            }
          }
        }
      }
    }

    if (mtoon1) {
      return MaterialFactory_MToon1(root, bin, *materialId, mtoon1);

    } else if (mtoon0) {
      return MaterialFactory_MToon0(root, bin, *materialId, mtoon0);
    }
  }

  return {};
}

} // namespace
