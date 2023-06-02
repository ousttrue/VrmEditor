#pragma once
#include "colorspace.h"
#include "material_factory.h"

namespace glr {

inline std::shared_ptr<MaterialFactory>
MaterialFactory_Unlit(const gltfjson::typing::Root& root,
                      const gltfjson::typing::Bin& bin,
                      std::optional<uint32_t> id)
{
  auto ptr = std::make_shared<MaterialFactory>();
  *ptr = MaterialFactory{
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
    ptr->FS.MacroGroups["AlphaBlend"].push_back({ u8"MODE_MASK" });
  }

  if (id) {
    auto src = root.Materials[*id];
    if (auto pbr = src.PbrMetallicRoughness()) {
      if (auto baseColorTexture = pbr->BaseColorTexture()) {
        if (auto texture = GetOrCreateTexture(
              root, bin, baseColorTexture->Index(), ColorSpace::sRGB)) {
          ptr->Textures.push_back({ 0, texture });
        }
      }
    }
  }

  return ptr;
}

}
