#pragma once
#include "app.h"
#include "gl3renderer.h"
#include "material_factory.h"
#include "shader_source.h"
#include <DirectXMath.h>
#include <gltfjson.h>
#include <grapho/gl3/pbr.h>

namespace glr {

inline DirectX::XMFLOAT3
Vec3(const gltfjson::tree::NodePtr& json, const DirectX::XMFLOAT3& defaultValue)
{
  if (json) {
    if (auto a = json->Array()) {
      if (a->size() == 3) {
        if (auto a0 = (*a)[0]) {
          if (auto p0 = a0->Ptr<float>()) {
            if (auto a1 = (*a)[1]) {
              if (auto p1 = a1->Ptr<float>()) {
                if (auto a2 = (*a)[2]) {
                  if (auto p2 = a2->Ptr<float>()) {
                    return { *p0, *p1, *p2 };
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return defaultValue;
}

inline std::shared_ptr<MaterialFactory>
MaterialFactory_Pbr_Khronos(const gltfjson::typing::Root& root,
                            const gltfjson::typing::Bin& bin,
                            std::optional<uint32_t> materialId)
{
  auto ptr = std::make_shared<MaterialFactory>();
  *ptr = MaterialFactory
  {
    .Type = ShaderTypes::Pbr,
    .VS={
      .SourceName = "khronos/primitive.vert",
      .Version=u8"#version 300 es",
      .MacroGroups{
        {"VERTEX", {
          { u8"HAS_NORMAL_VEC3", ConstInt(1) },
          { u8"HAS_POSITION_VEC3", ConstInt(1) },
          { u8"HAS_TEXCOORD_0_VEC2", ConstInt(1) },
        }},
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
          .Selected = { u8"DEBUG", ConstInt(0) },
        },
        {
          .Values = {
            { u8"ALPHAMODE_OPAQUE", 0 },
            { u8"ALPHAMODE_MASK", 1 },
            { u8"ALPHAMODE_BLEND", 2 },
          },
          .Selected = {u8"ALPHAMODE", IntVar{[](auto, auto, auto &json){
            gltfjson::typing::Material m(json);
            auto mode = m.AlphaMode();
            if(mode == u8"MASK")
            {
              return 1;
            }
            else if(mode == u8"BLEND")
            {
              return 2;
            }
            else{
              return 0;
            }
          }}},
        },
      },
      .MacroGroups{
        {"VertexAttribute", {
          {u8"HAS_POSITION_VEC3"},
          {u8"HAS_TEXCOORD_0_VEC2"},
          {u8"HAS_NORMAL_VEC3"},
          {u8"HAS_TANGENT_VEC4", Disable()},
          {u8"HAS_COLOR_0_VEC3", Disable()},
          {u8"HAS_COLOR_0_VEC4", Disable()},
        }},
        {"LIGHTING", {
          { u8"USE_PUNCTUAL" },
          { u8"LIGHT_COUNT", ConstInt(1) },
        }},
        {"Texture",{
          {u8"HAS_NORMAL_UV_TRANSFORM", Disable()},
          {u8"HAS_EMISSIVE_UV_TRANSFORM", Disable()},
          {u8"HAS_OCCLUSION_UV_TRANSFORM", Disable()},
          {u8"MATERIAL_METALLICROUGHNESS"},
          {u8"HAS_BASECOLOR_UV_TRANSFORM", Disable()},
          {u8"HAS_METALLICROUGHNESS_UV_TRANSFORM", Disable()},
          {u8"MATERIAL_SPECULARGLOSSINESS", Disable()},
          {u8"HAS_SPECULARGLOSSINESS_UV_TRANSFORM", Disable()},
          {u8"HAS_DIFFUSE_UV_TRANSFORM", Disable()},
          {u8"MATERIAL_CLEARCOAT", Disable()},
          {u8"HAS_CLEARCOAT_UV_TRANSFORM", Disable()},
          {u8"HAS_CLEARCOATROUGHNESS_UV_TRANSFORM", Disable()},
          {u8"HAS_CLEARCOATNORMAL_UV_TRANSFORM", Disable()},
          {u8"MATERIAL_SHEEN", Disable()},
          {u8"HAS_SHEENCOLOR_UV_TRANSFORM", Disable()},
          {u8"HAS_SHEENROUGHNESS_UV_TRANSFORM", Disable()},
          {u8"MATERIAL_SPECULAR", Disable()},
          {u8"HAS_SPECULAR_UV_TRANSFORM", Disable()},
          {u8"HAS_SPECULARCOLOR_UV_TRANSFORM", Disable()},
          {u8"MATERIAL_TRANSMISSION", Disable()},
          {u8"HAS_TRANSMISSION_UV_TRANSFORM", Disable()},
          {u8"MATERIAL_VOLUME", Disable()},
          {u8"HAS_THICKNESS_UV_TRANSFORM", Disable()},
          {u8"MATERIAL_IRIDESCENCE", Disable()},
          {u8"HAS_IRIDESCENCE_UV_TRANSFORM", Disable()},
          {u8"HAS_IRIDESCENCETHICKNESS_UV_TRANSFORM", Disable()},
        }},
        {"Material", {
          {u8"HAS_NORMAL_MAP", OptVar{[](auto, auto, auto &json)->std::optional<std::monostate>{
            gltfjson::typing::Material m(json);
            if (auto info = m.NormalTexture()) {
              return std::monostate{};
            }
            else{
              return std::nullopt;
            }
          }}},
          {u8"HAS_CLEARCOAT_NORMAL_MAP", Disable()},
          {u8"HAS_DIFFUSE_MAP", Disable()},

          {u8"HAS_SPECULAR_GLOSSINESS_MAP", Disable()},

          {u8"HAS_SHEEN_COLOR_MAP", Disable()},
          {u8"HAS_SHEEN_ROUGHNESS_MAP", Disable()},
          {u8"HAS_SPECULAR_MAP", Disable()},
          {u8"HAS_SPECULAR_COLOR_MAP", Disable()},
          {u8"HAS_TRANSMISSION_MAP", Disable()},
          {u8"HAS_THICKNESS_MAP", Disable()},
          {u8"HAS_IRIDESCENCE_MAP", Disable()},
          {u8"HAS_IRIDESCENCE_THICKNESS_MAP", Disable()},
          {u8"HAS_CLEARCOAT_MAP", Disable()},
          {u8"HAS_CLEARCOAT_ROUGHNESS_MAP", Disable()},
          {u8"MATERIAL_IOR", Disable()},
          {u8"USE_IBL", Disable()},

          {u8"HAS_BASE_COLOR_MAP", OptVar{[](auto, auto, auto &json)->std::optional<std::monostate>{
            gltfjson::typing::Material m(json);
            if(auto pbr = m.PbrMetallicRoughness())
            {
              if(pbr->BaseColorTexture())
              {
              return std::monostate{};
              }
            }
            return std::nullopt;
          }}},

          {u8"HAS_METALLIC_ROUGHNESS_MAP", OptVar{[](auto,auto,auto &json)->std::optional<std::monostate>{
            gltfjson::typing::Material m(json);
            if(auto pbr = m.PbrMetallicRoughness())
            {
              if(pbr->MetallicRoughnessTexture())
              {
              return std::monostate{};
              }
            }
            return std::nullopt;
          }}},

          {u8"HAS_EMISSIVE_MAP", OptVar{[](auto, auto, auto &json)->std::optional<std::monostate>{
            gltfjson::typing::Material m(json);
            if (auto info = m.EmissiveTexture()) {
              return std::monostate{};
            }
            return std::nullopt;
          }}},
          {u8"MATERIAL_EMISSIVE_STRENGTH", Disable()},

          {u8"HAS_OCCLUSION_MAP", OptVar{[](auto,auto,auto &json)->std::optional<std::monostate>{
            gltfjson::typing::Material m(json);
            if (auto info = m.OcclusionTexture()) {
              return std::monostate{};
            }
            return std::nullopt;
          }}},

          {u8"MATERIAL_UNLIT", OptVar{[](auto, auto, auto &json)->std::optional<std::monostate>{
            if(auto extensions = json->Get(u8"extensions"))
            {
              if(auto unlit = extensions->Get(u8"KHR_materials_unlit"))
              {
                return std::monostate{};
              }
            }
            return std::nullopt;
          }}},

          {u8"LINEAR_OUTPUT", Disable()},
        }},
      },
    },
    .UniformVarMap
    {
      {"u_BaseColorFactor",Vec4Var{[](auto &w, auto &l, auto){ return l.ColorRGBA(); }}},
      {"u_MetallicFactor",ConstFloat(1.0f)},
      {"u_RoughnessFactor",ConstFloat(1.0f)},
      {"u_Exposure",ConstFloat(1.0f)},

      {"u_ModelMatrix",Mat4Var{[](auto &w, auto &l, auto){ return l.ModelMatrix();}}},
      {"u_ViewProjectionMatrix",Mat4Var{[](auto &w, auto &l, auto){ return w.ViewProjectionMatrix();}}},

      {"u_EmissiveFactor",Vec3Var{[](auto &w, auto &l, auto &json){ 
        if(auto e = json->Get(u8"emissiveFactor"))
        {
          return Vec3(e, {0, 0, 0});
        }
        return DirectX::XMFLOAT3{0, 0, 0};
      }}},

      {"u_NormalMatrix",Mat4Var{[](auto &w, auto &l, auto){ return l.NormalMatrix4();}}},
      {"u_Camera",Vec3Var{[](auto &w, auto &l, auto){return w.CameraPosition();}}},

      {"u_Lights[0].color", Vec3Var{[](auto &w,auto, auto){
        auto &c=w.m_env.LightColor; 
        return DirectX::XMFLOAT3{c.x,c.y,c.z};
      }}},
      {"u_Lights[0].intensity", FloatVar{[](const WorldInfo &w, const LocalInfo &, auto){
        auto &c=w.m_env.LightColor;
        return c.w;
      }}},


      { "u_Lights[0].direction",
        Vec3Var{[](auto& w, auto, auto) {
          auto& p = w.m_env.LightPosition;
          return DirectX::XMFLOAT3{ -p.x, -p.y, -p.z };
        }} },
      { "u_Lights[0].position",
        Vec3Var{[](auto& w, auto, auto) {
          auto& p = w.m_env.LightPosition;
          return DirectX::XMFLOAT3{ p.x, p.y, p.z };
        }} },

      { "u_LambertianEnvSampler", ConstInt(0) }, { "u_GGXEnvSampler", ConstInt(1) },
      { "u_GGXLUT", ConstInt(2) }, { "u_CharlieEnvSampler", ConstInt(3) },
      { "u_CharlieLUT", ConstInt(4) }, { "u_SheenELUT", ConstInt(5) },
      { "u_NormalSampler", ConstInt(6) }, { "u_EmissiveSampler", ConstInt(7) },
      { "u_OcclusionSampler", ConstInt(8) }, { "u_BaseColorSampler", ConstInt(9) },
      { "u_MetallicRoughnessSampler", ConstInt(10) },

      { "u_NormalScale", FloatVar{[](auto, auto, auto &json){
        gltfjson::typing::Material m(json);
        if (auto normalTexture = m.NormalTexture()) {
          if(auto p = normalTexture->Scale())
          {
            return *p;
          }
        }
        return 1.0f;
      }}},

      { "u_AlphaCutoff", FloatVar{[](auto, auto, auto &json){
        gltfjson::typing::Material m(json);
        if(auto p = m.AlphaCutoff())
        {
          return *p;
        }
        else{
          return 0.5f;
        }
      }}},
    },
    .UpdateState = [](auto, auto, auto &json)
    {
      gltfjson::typing::Material m(json);
      auto mode = m.AlphaMode();
      if(mode == u8"MASK")
      {
        glDisable(GL_BLEND);
      }
      else if(mode == u8"BLEND")
      {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      }
      else{
        // OPAQUE
        glDisable(GL_BLEND);
      }

      bool *ds;
      if((ds = m.DoubleSided()) && *ds)
      {
        glDisable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
      }
      else{
        glEnable(GL_CULL_FACE);
      }
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
    ptr->Textures.push_back({ 6, normal });
  }
  if (emissive) {
    ptr->Textures.push_back({ 7, emissive });
  }
  if (ao) {
    ptr->Textures.push_back({ 8, ao });
  }
  if (albedo) {
    ptr->Textures.push_back({ 9, albedo });
  }
  if (metallic_roughness) {
    ptr->Textures.push_back({ 10, metallic_roughness });
  }

  return ptr;
}

} // namespace
