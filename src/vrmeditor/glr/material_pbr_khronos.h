#pragma once
#include "app.h"
#include "gl3renderer.h"
#include "material_factory.h"
#include "shader_source.h"
#include <gltfjson.h>
#include <grapho/gl3/material.h>
#include <grapho/gl3/pbr.h>

namespace glr {

inline std::shared_ptr<MaterialFactory>
MaterialFactory_Pbr_Khronos(
  const gltfjson::typing::Root& root,
  const gltfjson::typing::Bin& bin,
  std::optional<uint32_t> materialId)
{
  auto ptr = std::make_shared<MaterialFactory>();
  *ptr = MaterialFactory{
    .Type = ShaderTypes::Pbr,
    .VS={
      .SourceName = "khronos/primitive.vert",
      .Version=u8"#version 300 es",
    },
    .FS={
      .SourceName = "khronos/pbr.frag",
      .Version=u8"#version 300 es"
    },
  };

  std::shared_ptr<grapho::gl3::Texture> albedo;
  std::shared_ptr<grapho::gl3::Texture> metallic;
  std::shared_ptr<grapho::gl3::Texture> roughness;
  std::shared_ptr<grapho::gl3::Texture> normal;
  std::shared_ptr<grapho::gl3::Texture> ao;
  if (materialId) {
    auto src = root.Materials[*materialId];
    if (auto pbr = src.PbrMetallicRoughness()) {
      if (auto baseColorTexture = pbr->BaseColorTexture()) {
        albedo = GetOrCreateTexture(
          root, bin, baseColorTexture->Index(), ColorSpace::sRGB);
      }
      if (auto metallicRoughnessTexture = pbr->MetallicRoughnessTexture()) {
        metallic = GetOrCreateTexture(
          root, bin, metallicRoughnessTexture->Index(), ColorSpace::Linear);
        roughness = GetOrCreateTexture(
          root, bin, metallicRoughnessTexture->Index(), ColorSpace::Linear);
      }
    }
    if (auto normalTexture = src.NormalTexture()) {
      normal = GetOrCreateTexture(
        root, bin, normalTexture->Index(), ColorSpace::Linear);
    }
    if (auto occlusionTexture = src.OcclusionTexture()) {
      ao = GetOrCreateTexture(
        root, bin, occlusionTexture->Index(), ColorSpace::Linear);
    }
  }

  if (albedo) {
    ptr->FS.Macros.push_back({ u8"HAS_ALBEDO_TEXTURE" });
    ptr->Textures.push_back({ 0, albedo });
  }
  if (metallic) {
    ptr->FS.Macros.push_back({ u8"HAS_METALLIC_TEXTURE" });
    ptr->Textures.push_back({ 1, metallic });
  }
  if (roughness) {
    ptr->FS.Macros.push_back({ u8"HAS_ROUGHNESS_TEXTURE" });
    ptr->Textures.push_back({ 2, roughness });
  }
  if (ao) {
    ptr->FS.Macros.push_back({ u8"HAS_AO_TEXTURE" });
    ptr->Textures.push_back({ 3, ao });
  }
  if (normal) {
    ptr->FS.Macros.push_back({ u8"HAS_NORMAL_TEXTURE" });
    ptr->Textures.push_back({ 4, normal });
  }

  return ptr;
}

}
