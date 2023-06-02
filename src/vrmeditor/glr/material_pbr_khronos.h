#pragma once
#include "app.h"
#include "gl3renderer.h"
#include "material_factory.h"
#include "shader_source.h"
#include <gltfjson.h>
#include <grapho/gl3/pbr.h>

namespace glr {

// pbr.frag => tonemapping.glsl
// tonemapping.glsl: #ifdef TONEMAP_ACES_NARKOWICZ
// tonemapping.glsl: #ifdef TONEMAP_ACES_HILL
// tonemapping.glsl: #ifdef TONEMAP_ACES_HILL_EXPOSURE_BOOST
// pbr.frag => textures.glsl
// textures.glsl: #ifdef HAS_NORMAL_UV_TRANSFORM
// textures.glsl: #ifdef HAS_EMISSIVE_UV_TRANSFORM
// textures.glsl: #ifdef HAS_OCCLUSION_UV_TRANSFORM
// textures.glsl: #ifdef MATERIAL_METALLICROUGHNESS
// textures.glsl: #ifdef HAS_BASECOLOR_UV_TRANSFORM
// textures.glsl: #ifdef HAS_METALLICROUGHNESS_UV_TRANSFORM
// textures.glsl: #ifdef MATERIAL_SPECULARGLOSSINESS
// textures.glsl: #ifdef HAS_SPECULARGLOSSINESS_UV_TRANSFORM
// textures.glsl: #ifdef HAS_DIFFUSE_UV_TRANSFORM
// textures.glsl: #ifdef MATERIAL_CLEARCOAT
// textures.glsl: #ifdef HAS_CLEARCOAT_UV_TRANSFORM
// textures.glsl: #ifdef HAS_CLEARCOATROUGHNESS_UV_TRANSFORM
// textures.glsl: #ifdef HAS_CLEARCOATNORMAL_UV_TRANSFORM
// textures.glsl: #ifdef MATERIAL_SHEEN
// textures.glsl: #ifdef HAS_SHEENCOLOR_UV_TRANSFORM
// textures.glsl: #ifdef HAS_SHEENROUGHNESS_UV_TRANSFORM
// textures.glsl: #ifdef MATERIAL_SPECULAR
// textures.glsl: #ifdef HAS_SPECULAR_UV_TRANSFORM
// textures.glsl: #ifdef HAS_SPECULARCOLOR_UV_TRANSFORM
// textures.glsl: #ifdef MATERIAL_TRANSMISSION
// textures.glsl: #ifdef HAS_TRANSMISSION_UV_TRANSFORM
// textures.glsl: #ifdef MATERIAL_VOLUME
// textures.glsl: #ifdef HAS_THICKNESS_UV_TRANSFORM
// textures.glsl: #ifdef MATERIAL_IRIDESCENCE
// textures.glsl: #ifdef HAS_IRIDESCENCE_UV_TRANSFORM
// textures.glsl: #ifdef HAS_IRIDESCENCETHICKNESS_UV_TRANSFORM
// pbr.frag => functions.glsl
// functions.glsl: #ifdef HAS_NORMAL_VEC3
// functions.glsl: #ifdef HAS_TANGENT_VEC4
// functions.glsl: #ifdef HAS_COLOR_0_VEC3
// functions.glsl: #ifdef HAS_COLOR_0_VEC4
// pbr.frag => brdf.glsl
// pbr.frag => punctual.glsl
// punctual.glsl: #ifdef USE_PUNCTUAL
// pbr.frag => ibl.glsl
// pbr.frag => material_info.glsl
// material_info.glsl: #ifdef HAS_NORMAL_MAP
// material_info.glsl: #ifdef HAS_CLEARCOAT_NORMAL_MAP
// material_info.glsl: #if defined(MATERIAL_SPECULARGLOSSINESS)
// material_info.glsl: #elif defined(MATERIAL_METALLICROUGHNESS)
// material_info.glsl: #if defined(MATERIAL_SPECULARGLOSSINESS) && defined(HAS_DIFFUSE_MAP)        
// material_info.glsl: #elif defined(MATERIAL_METALLICROUGHNESS) && defined(HAS_BASE_COLOR_MAP)    
// material_info.glsl: #ifdef HAS_SPECULAR_GLOSSINESS_MAP
// material_info.glsl: #ifdef HAS_METALLIC_ROUGHNESS_MAP
// material_info.glsl: #ifdef HAS_SHEEN_COLOR_MAP
// material_info.glsl: #ifdef HAS_SHEEN_ROUGHNESS_MAP
// material_info.glsl: #ifdef HAS_SPECULAR_MAP
// material_info.glsl: #ifdef HAS_SPECULAR_COLOR_MAP
// material_info.glsl: #ifdef HAS_TRANSMISSION_MAP
// material_info.glsl: #ifdef HAS_THICKNESS_MAP
// material_info.glsl: #ifdef HAS_IRIDESCENCE_MAP
// material_info.glsl: #ifdef HAS_IRIDESCENCE_THICKNESS_MAP
// material_info.glsl: #ifdef HAS_CLEARCOAT_MAP
// material_info.glsl: #ifdef HAS_CLEARCOAT_ROUGHNESS_MAP
// material_info.glsl: #ifdef MATERIAL_IOR
// pbr.frag => iridescence.glsl
// pbr.frag: #if ALPHAMODE == ALPHAMODE_OPAQUE
// pbr.frag: #if DEBUG == DEBUG_METALLIC_ROUGHNESS
// pbr.frag: #ifdef USE_IBL
// pbr.frag: #if defined(MATERIAL_TRANSMISSION) && defined(USE_IBL)
// pbr.frag: #ifdef MATERIAL_EMISSIVE_STRENGTH
// pbr.frag: #ifdef HAS_EMISSIVE_MAP
// pbr.frag: #ifdef HAS_OCCLUSION_MAP
// pbr.frag: #ifdef MATERIAL_UNLIT
// pbr.frag: #if DEBUG == DEBUG_NONE
// pbr.frag: #if ALPHAMODE == ALPHAMODE_MASK
// pbr.frag: #ifdef LINEAR_OUTPUT
// pbr.frag: #if DEBUG == DEBUG_UV_0 && defined(HAS_TEXCOORD_0_VEC2)
// pbr.frag: #if DEBUG == DEBUG_UV_1 && defined(HAS_TEXCOORD_1_VEC2)
// pbr.frag: #if DEBUG == DEBUG_NORMAL_TEXTURE && defined(HAS_NORMAL_MAP)
// pbr.frag: #if DEBUG == DEBUG_NORMAL_SHADING
// pbr.frag: #if DEBUG == DEBUG_NORMAL_GEOMETRY
// pbr.frag: #if DEBUG == DEBUG_TANGENT
// pbr.frag: #if DEBUG == DEBUG_BITANGENT
// pbr.frag: #if DEBUG == DEBUG_ALPHA
// pbr.frag: #if DEBUG == DEBUG_OCCLUSION && defined(HAS_OCCLUSION_MAP)
// pbr.frag: #if DEBUG == DEBUG_EMISSIVE
// pbr.frag: #if DEBUG == DEBUG_METALLIC
// pbr.frag: #if DEBUG == DEBUG_ROUGHNESS
// pbr.frag: #if DEBUG == DEBUG_BASE_COLOR
// pbr.frag: #if DEBUG == DEBUG_CLEARCOAT
// pbr.frag: #if DEBUG == DEBUG_CLEARCOAT_FACTOR
// pbr.frag: #if DEBUG == DEBUG_CLEARCOAT_ROUGHNESS
// pbr.frag: #if DEBUG == DEBUG_CLEARCOAT_NORMAL
// pbr.frag: #if DEBUG == DEBUG_SHEEN
// pbr.frag: #if DEBUG == DEBUG_SHEEN_COLOR
// pbr.frag: #if DEBUG == DEBUG_SHEEN_ROUGHNESS
// pbr.frag: #if DEBUG == DEBUG_SPECULAR
// pbr.frag: #if DEBUG == DEBUG_SPECULAR_FACTOR
// pbr.frag: #if DEBUG == DEBUG_SPECULAR_COLOR
// pbr.frag: #if DEBUG == DEBUG_TRANSMISSION_VOLUME
// pbr.frag: #if DEBUG == DEBUG_TRANSMISSION_FACTOR
// pbr.frag: #if DEBUG == DEBUG_VOLUME_THICKNESS
// pbr.frag: #if DEBUG == DEBUG_IRIDESCENCE
// pbr.frag: #if DEBUG == DEBUG_IRIDESCENCE_FACTOR
// pbr.frag: #if DEBUG == DEBUG_IRIDESCENCE_THICKNESS

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
      .Macros{
        { u8"HAS_NORMAL_VEC3", 1 },
        { u8"HAS_POSITION_VEC3", 1 },
        { u8"HAS_TEXCOORD_0_VEC2", 1 },
      },
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
        },
        {
          .Values = {
            { u8"ALPHAMODE_OPAQUE", 0 },
            { u8"ALPHAMODE_MASK", 1 },
            { u8"ALPHAMODE_BLEND", 2 },
          },
          .Selected = {u8"ALPHAMODE", 0},
        },
      },
      .Macros = {
        { u8"MATERIAL_METALLICROUGHNESS", 1 },
        { u8"HAS_NORMAL_VEC3", 1 },
        { u8"HAS_POSITION_VEC3", 1 },
        { u8"HAS_TEXCOORD_0_VEC2", 1 },
        { u8"USE_PUNCTUAL", 1 },
        { u8"LIGHT_COUNT", 1 },
      },
    },
    .UniformGetterMap
    {
      {"u_BaseColorFactor",[](auto &w, auto &l, auto){ return l.ColorRGBA(); }},
      {"u_MetallicFactor",GetFloat(1.0f)},
      {"u_RoughnessFactor",GetFloat(1.0f)},
      {"u_Exposure",GetFloat(1.0f)},

      {"u_ModelMatrix",[](auto &w, auto &l, auto){ return l.ModelMatrix();}},
      {"u_ViewProjectionMatrix",[](auto &w, auto &l, auto){ return w.ViewProjectionMatrix();}},
      {"u_EmissiveFactor",[](auto &w, auto &l, auto){ return l.EmissiveRGB();}},
      {"u_NormalMatrix",[](auto &w, auto &l, auto){ return l.NormalMatrix4();}},
      {"u_Camera",[](auto &w, auto &l, auto){return w.CameraPosition();}},

      {"u_Lights[0].color", [](auto &w,auto, auto){
        auto &c=w.m_env.LightColor; 
        return DirectX::XMFLOAT3{c.x,c.y,c.z};
      }},
      {"u_Lights[0].intensity", (GetterFunc<float>)[](const WorldInfo &w, const LocalInfo &, auto){
        auto &c=w.m_env.LightColor; 
        return c.w;
      }},
      {"u_Lights[0].direction", [](auto &w,auto, auto){
        auto &p=w.m_env.LightPosition; 
        return DirectX::XMFLOAT3{-p.x,-p.y,-p.z};
      }},
      {"u_Lights[0].position", [](auto &w,auto, auto){
        auto &p=w.m_env.LightPosition; 
        return DirectX::XMFLOAT3{p.x,p.y,p.z};
      }},

      {"u_LambertianEnvSampler",GetInt(0)},
      {"u_GGXEnvSampler",GetInt(1)},
      {"u_GGXLUT",GetInt(2)},
      {"u_CharlieEnvSampler",GetInt(3)},
      {"u_CharlieLUT",GetInt(4)},
      {"u_SheenELUT",GetInt(5)},
      {"u_NormalSampler",GetInt(6)},
      {"u_EmissiveSampler",GetInt(7)},
      {"u_OcclusionSampler",GetInt(8)},
      {"u_BaseColorSampler",GetInt(9)},
      {"u_MetallicRoughnessSampler",GetInt(10)},
     },
  };

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
