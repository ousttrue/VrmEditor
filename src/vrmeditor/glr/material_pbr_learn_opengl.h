#pragma once
#include "material_factory.h"

namespace glr {

inline std::shared_ptr<MaterialFactory>
MaterialFactory_Pbr_LearnOpenGL(const gltfjson::typing::Root& root,
                                const gltfjson::typing::Bin& bin,
                                std::optional<uint32_t> materialId)
{
  auto ptr = std::make_shared<MaterialFactory>();
  *ptr = MaterialFactory{
    .Type = ShaderTypes::Pbr,
    .VS = {
      .SourceName ="pbr.vert",
      .Version = u8"#version 450",
    },
    .FS = {
      .SourceName ="pbr.frag",
      .Version = u8"#version 450",
    },
    .Updater = [](auto &shader, auto &env, auto &draw, auto &shadow)
    {
      if (auto var = shader->Uniform("irradianceMap")) {
        var->SetInt(0);
      }
      if (auto var = shader->Uniform("prefilterMap")) {
        var->SetInt(1);
      }
      if (auto var = shader->Uniform("brdfLUT")) {
        var->SetInt(2);
      }
      if (auto var = shader->Uniform("albedoMap")) {
        var->SetInt(3);
      }
      if (auto var = shader->Uniform("normalMap")) {
        var->SetInt(4);
      }
      if (auto var = shader->Uniform("metallicMap")) {
        var->SetInt(5);
      };
      if (auto var = shader->Uniform("roughnessMap")) {
        var->SetInt(6);
      }
      if (auto var = shader->Uniform("aoMap")) {
        var->SetInt(7);
      }
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
    ptr->FS.Macros.push_back({ u8"HAS_ALBEDO_TEXTURE" });
    ptr->Textures.push_back({ 3, albedo });
  }
  if (normal) {
    ptr->FS.Macros.push_back({ u8"HAS_NORMAL_TEXTURE" });
    ptr->Textures.push_back({ 4, normal });
  }
  if (metallic) {
    ptr->FS.Macros.push_back({ u8"HAS_METALLIC_TEXTURE" });
    ptr->Textures.push_back({ 5, metallic });
  }
  if (roughness) {
    ptr->FS.Macros.push_back({ u8"HAS_ROUGHNESS_TEXTURE" });
    ptr->Textures.push_back({ 6, roughness });
  }
  if (ao) {
    ptr->FS.Macros.push_back({ u8"HAS_AO_TEXTURE" });
    ptr->Textures.push_back({ 7, ao });
  }

  return ptr;
}

}
