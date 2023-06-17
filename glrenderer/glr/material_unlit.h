#pragma once
#include "colorspace.h"
#include "material.h"

namespace glr {

inline std::shared_ptr<Material>
MaterialFactory_Unlit(const gltfjson::Root& root,
                      const gltfjson::Bin& bin,
                      std::optional<uint32_t> id)
{
  auto ptr = std::make_shared<Material>();
  *ptr = Material{
    .Name = "experimental simple unlit",
    .VS = {
      .SourceName = "unlit.vert",
      .Version = u8"#version 450",
    },
    .FS = {
      .SourceName= "unlit.frag",
      .Version = u8"#version 450",
    },
  };

  // update world UBO
  // m_world.projection = env.ProjectionMatrix;
  // m_world.view = env.ViewMatrix;
  // m_world.camPos = {
  //   env.CameraPosition.x,
  //   env.CameraPosition.y,
  //   env.CameraPosition.z,
  //   1,
  // };

  // update local UBO
  // auto gltfMaterial = root.Materials[*primitive.Material];
  // if (auto cutoff = gltfMaterial.AlphaCutoff()) {
  //   m_local.cutoff.x = *cutoff;
  // }
  // m_local.color = { 1, 1, 1, 1 };
  // if (auto pbr = gltfMaterial.PbrMetallicRoughness()) {
  //   if (pbr->BaseColorFactor.size() == 4) {
  //     m_local.color.x = pbr->BaseColorFactor[0];
  //     m_local.color.y = pbr->BaseColorFactor[1];
  //     m_local.color.z = pbr->BaseColorFactor[2];
  //     m_local.color.w = pbr->BaseColorFactor[3];
  //   }
  // }
  //

  // if (gltfjson::GetAlphaMode(root, id) ==
  //     gltfjson::format::AlphaModes::Mask) {
  //   ptr->FS.MacroGroups["AlphaBlend"].push_back({ u8"MODE_MASK" });
  // }

  if (id) {
    auto src = root.Materials[*id];
    if (auto pbr = src.PbrMetallicRoughness()) {
      if (auto baseColorTexture = pbr->BaseColorTexture()) {
        if (auto texture = GetOrCreateTexture(
              root, bin, baseColorTexture->IndexId(), ColorSpace::sRGB)) {
          ptr->Textures.push_back({ 0, texture });
        }
      }
    }
  }

  return ptr;
}

}
