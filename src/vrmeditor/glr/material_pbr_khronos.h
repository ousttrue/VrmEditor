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
MaterialFactory_Pbr_Khronos(const gltfjson::typing::Root& root,
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
      .Version=u8"#version 300 es",
      .Enums = {
        {
          .Values = {
            { u8"DEBUG_NONE", 0 },
            { u8"DEBUG_NORMAL_SHADING", 1 },
            { u8"DEBUG_NORMAL_TEXTURE", 2 },
            { u8"DEBUG_NORMAL_GEOMETRY", 3 },
            { u8"DEBUG_TANGENT", 4 },
            { u8"DEBUG_BITANGENT", 5 },
            { u8"DEBUG_ALPHA", 6 },
            { u8"DEBUG_UV_0", 7 },
            { u8"DEBUG_UV_1", 8 },
            { u8"DEBUG_OCCLUSION", 9 },
            { u8"DEBUG_EMISSIVE", 10 },
            { u8"DEBUG_METALLIC_ROUGHNESS", 11 },
            { u8"DEBUG_BASE_COLOR", 12 },
            { u8"DEBUG_ROUGHNESS", 13 },
            { u8"DEBUG_METALLIC", 14 },
            { u8"DEBUG_CLEARCOAT", 15 },
            { u8"DEBUG_CLEARCOAT_FACTOR", 16 },
            { u8"DEBUG_CLEARCOAT_ROUGHNESS", 17 },
            { u8"DEBUG_CLEARCOAT_NORMAL", 18 },
            { u8"DEBUG_SHEEN", 19 },
            { u8"DEBUG_SHEEN_COLOR", 20 },
            { u8"DEBUG_SHEEN_ROUGHNESS", 21 },
            { u8"DEBUG_SPECULAR", 22 },
            { u8"DEBUG_SPECULAR_FACTOR", 23 },
            { u8"DEBUG_SPECULAR_COLOR", 24 },
            { u8"DEBUG_TRANSMISSION_VOLUME", 25 },
            { u8"DEBUG_TRANSMISSION_FACTOR", 26 },
            { u8"DEBUG_VOLUME_THICKNESS", 27 },
            { u8"DEBUG_IRIDESCENCE", 28 },
            { u8"DEBUG_IRIDESCENCE_FACTOR", 29 },
            { u8"DEBUG_IRIDESCENCE_THICKNESS", 30 },
            { u8"DEBUG_ANISOTROPIC_STRENGTH", 31 },
            { u8"DEBUG_ANISOTROPIC_DIRECTION", 32 },
          },
          .Selected = { u8"DEBUG", 0 },
        }
      },
    },
    .Updater = [](auto &shader, auto &env, auto &draw, auto &shadow)
    {
      shader->SetUniform("u_ModelMatrix",draw.model);
      shader->SetUniform("u_ViewProjectionMatrix",env.viewprojection());
      shader->SetUniform("u_EmissiveFactor",DirectX::XMFLOAT3{1,1,1});
      shader->SetUniform("u_NormalMatrix",draw.normalMatrix);

      shader->SetUniform("u_LambertianEnvSampler",0);
      shader->SetUniform("u_GGXEnvSampler",1);
      shader->SetUniform("u_GGXLUT",2);
      shader->SetUniform("u_CharlieEnvSampler",3);
      shader->SetUniform("u_CharlieLUT",4);
      shader->SetUniform("u_SheenELUT",5);
      shader->SetUniform("u_NormalSampler",6);
      shader->SetUniform("u_EmissiveSampler",7);
      shader->SetUniform("u_OcclusionSampler",8);
      shader->SetUniform("u_BaseColorSampler",9);
      shader->SetUniform("u_MetallicRoughnessSampler",10);
     },
  };

  //
  // vs
  //
  ptr->VS.Macros.push_back({ u8"HAS_NORMAL_VEC3", 1 });
  ptr->VS.Macros.push_back({ u8"HAS_POSITION_VEC3", 1 });
  ptr->VS.Macros.push_back({ u8"HAS_TEXCOORD_0_VEC2", 1 });

  //
  // fs
  //
  std::shared_ptr<grapho::gl3::Texture> albedo;
  std::shared_ptr<grapho::gl3::Texture> metallic_roughness;
  std::shared_ptr<grapho::gl3::Texture> normal;
  std::shared_ptr<grapho::gl3::Texture> ao;
  std::shared_ptr<grapho::gl3::Texture> emissive;
  if (materialId) {
    auto src = root.Materials[*materialId];
    if (auto pbr = src.PbrMetallicRoughness()) {
      if (auto baseColorTexture = pbr->BaseColorTexture()) {
        albedo = GetOrCreateTexture(
          root, bin, baseColorTexture->Index(), ColorSpace::sRGB);
      }
      if (auto metallicRoughnessTexture = pbr->MetallicRoughnessTexture()) {
        metallic_roughness = GetOrCreateTexture(
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
    if (auto emissiveTexture = src.EmissiveTexture()) {
      emissive = GetOrCreateTexture(
        root, bin, emissiveTexture->Index(), ColorSpace::sRGB);
    }
  }

  ptr->FS.Macros.push_back({ u8"ALPHAMODE_OPAQUE", 0 });
  ptr->FS.Macros.push_back({ u8"ALPHAMODE_MASK", 1 });
  ptr->FS.Macros.push_back({ u8"ALPHAMODE_BLEND", 2 });
  ptr->FS.Macros.push_back({ u8"ALPHAMODE", u8"ALPHAMODE_OPAQUE" });
  ptr->FS.Macros.push_back({ u8"MATERIAL_METALLICROUGHNESS", 1 });
  ptr->FS.Macros.push_back({ u8"HAS_NORMAL_VEC3", 1 });
  ptr->FS.Macros.push_back({ u8"HAS_POSITION_VEC3", 1 });
  ptr->FS.Macros.push_back({ u8"HAS_TEXCOORD_0_VEC2", 1 });
  ptr->FS.Macros.push_back({ u8"USE_PUNCTUAL", 1 });
  ptr->FS.Macros.push_back({ u8"LIGHT_COUNT", 0 });
  // "USE_IBL 1"

  if (normal) {
    ptr->FS.Macros.push_back({ u8"HAS_NORMAL_MAP", 1 });
    ptr->Textures.push_back({ 6, normal });
  }
  if (emissive) {
    ptr->FS.Macros.push_back({ u8"HAS_EMISSIVE_MAP", 1 });
    ptr->Textures.push_back({ 7, emissive });
  }
  if (ao) {
    ptr->FS.Macros.push_back({ u8"HAS_OCCLUSION_MAP", 1 });
    ptr->Textures.push_back({ 8, ao });
  }

  if (albedo) {
    ptr->FS.Macros.push_back({ u8"HAS_BASE_COLOR_MAP", 1 });
    ptr->Textures.push_back({ 9, albedo });
  }
  if (metallic_roughness) {
    ptr->FS.Macros.push_back({ u8"HAS_METALLIC_ROUGHNESS_MAP", 1 });
    ptr->Textures.push_back({ 10, metallic_roughness });
  }

  return ptr;
}

}
