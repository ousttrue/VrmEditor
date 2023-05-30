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
      .Version=u8"#version 300 es"
    },
    .Updater = [](auto &shader, auto &env, auto &draw, auto &shadow)
    {
      shader->Uniform("u_ModelMatrix")->SetMat4(draw.model);
      shader->Uniform("u_ViewProjectionMatrix")->SetMat4(env.viewprojection());
      shader->Uniform("u_EmissiveFactor")->SetFloat3(DirectX::XMFLOAT3{1,1,1});
      // shader->Uniform("u_NormalMatrix")->SetMat4(draw.normalMatrix);

      if (auto var = shader->Uniform("u_LambertianEnvSampler")) {
        //cube
        var->SetInt(0);
      }
      if (auto var = shader->Uniform("u_GGXEnvSampler")) {
        //cube
        var->SetInt(1);
      }
      if (auto var = shader->Uniform("u_GGXLUT")) {
        var->SetInt(2);
      }
      if (auto var = shader->Uniform("u_CharlieEnvSampler")) {
        //cube
        var->SetInt(3);
      }
      if (auto var = shader->Uniform("u_CharlieLUT")) {
        var->SetInt(4);
      }
      if (auto var = shader->Uniform("u_SheenELUT")) {
        var->SetInt(5);
      };
      if (auto var = shader->Uniform("u_NormalSampler")) {
        var->SetInt(6);
      }
      if (auto var = shader->Uniform("u_EmissiveSampler")) {
        var->SetInt(7);
      }
      if (auto var = shader->Uniform("u_OcclusionSampler")) {
        var->SetInt(8);
      }
       if (auto var = shader->Uniform("u_BaseColorSampler")) {
        var->SetInt(9);
      }
       if (auto var = shader->Uniform("u_MetallicRoughnessSampler")) {
        var->SetInt(10);
      }
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

  ptr->FS.Macros.push_back({ u8"DEBUG_NONE", 0 });
  ptr->FS.Macros.push_back({ u8"DEBUG_NORMAL_SHADING", 1 });
  ptr->FS.Macros.push_back({ u8"DEBUG_NORMAL_TEXTURE", 2 });
  ptr->FS.Macros.push_back({ u8"DEBUG_NORMAL_GEOMETRY", 3 });
  ptr->FS.Macros.push_back({ u8"DEBUG_TANGENT", 4 });
  ptr->FS.Macros.push_back({ u8"DEBUG_BITANGENT", 5 });
  ptr->FS.Macros.push_back({ u8"DEBUG_ALPHA", 6 });
  ptr->FS.Macros.push_back({ u8"DEBUG_UV_0", 7 });
  ptr->FS.Macros.push_back({ u8"DEBUG_UV_1", 8 });
  ptr->FS.Macros.push_back({ u8"DEBUG_OCCLUSION", 9 });
  ptr->FS.Macros.push_back({ u8"DEBUG_EMISSIVE", 10 });
  ptr->FS.Macros.push_back({ u8"DEBUG_METALLIC_ROUGHNESS", 11 });
  ptr->FS.Macros.push_back({ u8"DEBUG_BASE_COLOR", 12 });
  ptr->FS.Macros.push_back({ u8"DEBUG_ROUGHNESS", 13 });
  ptr->FS.Macros.push_back({ u8"DEBUG_METALLIC", 14 });
  ptr->FS.Macros.push_back({ u8"DEBUG_CLEARCOAT", 15 });
  ptr->FS.Macros.push_back({ u8"DEBUG_CLEARCOAT_FACTOR", 16 });
  ptr->FS.Macros.push_back({ u8"DEBUG_CLEARCOAT_ROUGHNESS", 17 });
  ptr->FS.Macros.push_back({ u8"DEBUG_CLEARCOAT_NORMAL", 18 });
  ptr->FS.Macros.push_back({ u8"DEBUG_SHEEN", 19 });
  ptr->FS.Macros.push_back({ u8"DEBUG_SHEEN_COLOR", 20 });
  ptr->FS.Macros.push_back({ u8"DEBUG_SHEEN_ROUGHNESS", 21 });
  ptr->FS.Macros.push_back({ u8"DEBUG_SPECULAR", 22 });
  ptr->FS.Macros.push_back({ u8"DEBUG_SPECULAR_FACTOR", 23 });
  ptr->FS.Macros.push_back({ u8"DEBUG_SPECULAR_COLOR", 24 });
  ptr->FS.Macros.push_back({ u8"DEBUG_TRANSMISSION_VOLUME", 25 });
  ptr->FS.Macros.push_back({ u8"DEBUG_TRANSMISSION_FACTOR", 26 });
  ptr->FS.Macros.push_back({ u8"DEBUG_VOLUME_THICKNESS", 27 });
  ptr->FS.Macros.push_back({ u8"DEBUG_IRIDESCENCE", 28 });
  ptr->FS.Macros.push_back({ u8"DEBUG_IRIDESCENCE_FACTOR", 29 });
  ptr->FS.Macros.push_back({ u8"DEBUG_IRIDESCENCE_THICKNESS", 30 });
  ptr->FS.Macros.push_back({ u8"DEBUG_ANISOTROPIC_STRENGTH", 31 });
  ptr->FS.Macros.push_back({ u8"DEBUG_ANISOTROPIC_DIRECTION", 32 });
  ptr->FS.Macros.push_back({ u8"DEBUG", 1 });

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
