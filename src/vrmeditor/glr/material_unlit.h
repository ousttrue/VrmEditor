#pragma once
#include "material_factory.h"

namespace glr {

inline MaterialFactory
MaterialFactory_Unlit(const gltfjson::typing::Root& root,
                      const gltfjson::typing::Bin& bin,
                      std::optional<uint32_t> id)
{
  MaterialFactory factory{
    .Type =ShaderTypes::Unlit,
    .VS = {
      .SourceName = "unlit.vert",
      .Version = u8"#version 450",
    },
    .FS = {
      .SourceName= "unlit.frag",
      .Version = u8"#version 450",
    },
  };

  if (gltfjson::typing::GetAlphaMode(root, id) ==
      gltfjson::format::AlphaModes::Mask) {
    factory.FS.Macros.push_back({ u8"MODE_MASK" });
  }

  if (id) {
    auto src = root.Materials[*id];
    if (auto pbr = src.PbrMetallicRoughness()) {
      if (auto baseColorTexture = pbr->BaseColorTexture()) {
        if (auto texture = GetOrCreateTexture(
              root, bin, baseColorTexture->Index(), ColorSpace::sRGB)) {
          factory.Textures.push_back({ 0, texture });
        }
      }
    }
  }

  return factory;
  // return MaterialWithUpdater{ material };
  // } else {
  //   App::Instance().Log(LogLevel::Error) << shader.error();
  //   return std::unexpected{ shader.error() };
  // }
}

}
