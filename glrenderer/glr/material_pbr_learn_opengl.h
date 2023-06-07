#pragma once
#include "material.h"

namespace glr {

inline std::shared_ptr<Material>
MaterialFactory_Pbr_LearnOpenGL(const gltfjson::Root& root,
                                const gltfjson::Bin& bin,
                                std::optional<uint32_t> materialId)
{
  auto ptr = std::make_shared<Material>();
  *ptr = Material{
    .Name = "PBR based Learn OpenGL",
    .VS = {
      .SourceName ="pbr.vert",
      .Version = u8"#version 450",
    },
    .FS = {
      .SourceName ="pbr.frag",
      .Version = u8"#version 450",
    },
    .EnvCubemaps{
      {0, EnvCubemapTypes::LOGL_IrradianceMap},
      {1, EnvCubemapTypes::LOGL_PrefilterMap},
    },
    .EnvTextures{
      {2, EnvTextureTypes::LOGL_BrdfLUT},
    },
    .UniformVarMap=
    {
      {"irradianceMap", ConstInt(0)},
      {"prefilterMap", ConstInt(1)},
      {"brdfLUT",ConstInt(2)},
      {"albedoMap",ConstInt(3)},
      {"normalMap",ConstInt(4)},
      {"metallicMap",ConstInt(5)},
      {"roughnessMap",ConstInt(6)},
      {"aoMap",ConstInt(7)},
    },
  };

  std::shared_ptr<grapho::gl3::Texture> albedo;
  std::shared_ptr<grapho::gl3::Texture> metallic;
  std::shared_ptr<grapho::gl3::Texture> roughness;
  std::shared_ptr<grapho::gl3::Texture> ao;
  std::shared_ptr<grapho::gl3::Texture> normal;
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
    ptr->FS.MacroGroups["Texture"].push_back({ u8"HAS_ALBEDO_TEXTURE" });
    ptr->Textures.push_back({ 3, albedo });
  }
  if (normal) {
    ptr->FS.MacroGroups["Texture"].push_back({ u8"HAS_NORMAL_TEXTURE" });
    ptr->Textures.push_back({ 4, normal });
  }
  if (metallic) {
    ptr->FS.MacroGroups["Texture"].push_back({ u8"HAS_METALLIC_TEXTURE" });
    ptr->Textures.push_back({ 5, metallic });
  }
  if (roughness) {
    ptr->FS.MacroGroups["Texture"].push_back({ u8"HAS_ROUGHNESS_TEXTURE" });
    ptr->Textures.push_back({ 6, roughness });
  }
  if (ao) {
    ptr->FS.MacroGroups["Texture"].push_back({ u8"HAS_AO_TEXTURE" });
    ptr->Textures.push_back({ 7, ao });
  }

  return ptr;
}

}
