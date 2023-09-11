#include <GL/glew.h>

#include "material_three_vrm0.h"
#include "material_three_vrm_builtin.h"
#include <gltfjson/gltf_typing_vrm0.h>

static DirectX::XMFLOAT4X4
mult(const DirectX::XMFLOAT4X4& lhs, const DirectX::XMFLOAT4X4& rhs)
{
  DirectX::XMFLOAT4X4 m;
  DirectX::XMStoreFloat4x4(
    &m, DirectX::XMLoadFloat4x4(&lhs) * DirectX::XMLoadFloat4x4(&rhs));
  return m;
}

// { u8"DEBUG_LITSHADERATE" },
// { u8"DEBUG_UV" },
// { u8"DEBUG_NORMAL" },
// { u8"USE_SHADINGSHIFTTEXTURE"},
// { u8"USE_RIMMULTIPLYTEXTURE"},
// { u8"USE_UVANIMATIONMASKTEXTURE"},
// color_pars_fragment: #if defined( USE_COLOR_ALPHA )
// color_pars_fragment: #elif defined( USE_COLOR )
// aomap_pars_fragment: #ifdef USE_AOMAP
// emissivemap_pars_fragment: #ifdef USE_EMISSIVEMAP
// lights_pars_begin: #if defined ( LEGACY_LIGHTS )
// lights_pars_begin: #if NUM_DIR_LIGHTS > 0
// lights_pars_begin: #if NUM_POINT_LIGHTS > 0
// lights_pars_begin: #if NUM_SPOT_LIGHTS > 0
// lights_pars_begin: #if NUM_RECT_AREA_LIGHTS > 0
// lights_pars_begin: #if NUM_HEMI_LIGHTS > 0
// normal_pars_fragment: #ifdef USE_TANGENT
// mtoon.frag: #ifdef USE_NORMALMAP
// mtoon.frag: #ifdef OBJECTSPACE_NORMALMAP
// mtoon.frag: #ifdef DOUBLE_SIDED
// mtoon.frag: #ifdef OUTLINE
// mtoon.frag: #ifdef FLIP_SIDED
// lights_fragment_maps: #ifdef USE_LIGHTMAP
// lights_fragment_maps: #if defined( USE_ENVMAP ) && defined( STANDARD ) &&
// defined( ENVMAP_TYPE_CUBE_UV ) lights_fragment_maps: #if defined( USE_ENVMAP
// ) && defined( RE_IndirectSpecular ) aomap_fragment: #if defined( USE_ENVMAP )
// && defined( STANDARD ) mtoon.frag: #if defined( OUTLINE )

// _Cutoff ("Alpha Cutoff", Range(0, 1)) = 0.5
// _Color ("Lit Color + Alpha", Color) = (1,1,1,1)
// _ShadeColor ("Shade Color", Color) = (0.97, 0.81, 0.86, 1)
// _BumpScale ("Normal Scale", Float) = 1.0
// _ReceiveShadowRate ("Receive Shadow", Range(0, 1)) = 1
// _ShadingGradeRate ("Shading Grade", Range(0, 1)) = 1
// _ShadeShift ("Shade Shift", Range(-1, 1)) = 0
// _ShadeToony ("Shade Toony", Range(0, 1)) = 0.9
// _LightColorAttenuation ("Light Color Attenuation", Range(0, 1)) = 0
// _IndirectLightIntensity ("Indirect Light Intensity", Range(0, 1)) = 0.1
// [HDR] _RimColor ("Rim Color", Color) = (0,0,0)
// _RimLightingMix ("Rim Lighting Mix", Range(0, 1)) = 0
// [PowerSlider(4.0)] _RimFresnelPower ("Rim Fresnel Power", Range(0, 100)) = 1
// _RimLift ("Rim Lift", Range(0, 1)) = 0
// [HDR] _EmissionColor ("Color", Color) = (0,0,0)
// _OutlineWidth ("Outline Width", Range(0.01, 1)) = 0.5
// _OutlineScaledMaxDistance ("Outline Scaled Max Distance", Range(1, 10)) = 1
// _OutlineColor ("Outline Color", Color) = (0,0,0,1)
// _OutlineLightingMix ("Outline Lighting Mix", Range(0, 1)) = 1
// _UvAnimScrollX ("UV Animation Scroll X", Float) = 0
// _UvAnimScrollY ("UV Animation Scroll Y", Float) = 0
// _UvAnimRotation ("UV Animation Rotation", Float) = 0
// [NoScaleOffset] _MainTex ("Lit Texture + Alpha", 2D) = "white" {}
// [NoScaleOffset] _ShadeTexture ("Shade Texture", 2D) = "white" {}
// [Normal] _BumpMap ("Normal Texture", 2D) = "bump" {}
// [NoScaleOffset] _ReceiveShadowTexture ("Receive Shadow Texture", 2D) =
// "white" {} [NoScaleOffset] _ShadingGradeTexture ("Shading Grade Texture", 2D)
// = "white" {} [NoScaleOffset] _RimTexture ("Rim Texture", 2D) = "white" {}
// [NoScaleOffset] _SphereAdd ("Sphere Texture(Add)", 2D) = "black" {}
// [NoScaleOffset] _EmissionMap ("Emission", 2D) = "white" {}
// [NoScaleOffset] _OutlineWidthTexture ("Outline Width Tex", 2D) = "white" {}
// [NoScaleOffset] _UvAnimMaskTexture ("UV Animation Mask", 2D) = "white" {}
//   uniform sampler2D aoMap;
//   uniform sampler2D ltc_1; // RGBA Float
//   uniform sampler2D ltc_2; // RGBA Float
//   uniform sampler2D spotLightMap[NUM_SPOT_LIGHT_MAPS];
//   uniform sampler2D directionalShadowMap[NUM_DIR_LIGHT_SHADOWS];
//   uniform sampler2D spotShadowMap[NUM_SPOT_LIGHT_SHADOWS];
//   uniform sampler2D pointShadowMap[NUM_POINT_LIGHT_SHADOWS];

namespace glr {

std::shared_ptr<Material>
MaterialFactory_MToon0(const gltfjson::Root& root,
                       const gltfjson::Bin& bin,
                       uint32_t materialId,
                       const gltfjson::tree::NodePtr& mtoon0)
{
  auto ptr = std::make_shared<Material>();

  *ptr = Material{
    .Name = "three-vrm(vrm-0.x)",
    .VS={
      .SourceName = "mtoon.vert",
      .Version = "#version 300 es",
      .Codes{ VS_BUILTIN },
      .MacroGroups{
        {"VS", {
          { "THREE_VRM_THREE_REVISION", ConstInt(150) },
          { "NUM_SPOT_LIGHT_COORDS", ConstInt(4) },
          { "NUM_CLIPPING_PLANES", ConstInt(0) },
          { "MTOON_USE_UV" },
        }},
      },
    },
    .FS={
      .SourceName = "mtoon.frag",
      .Version ="#version 300 es",
      .Precision = "mediump float",
      .Codes{ FS_BUIlTIN },
      .MacroGroups{
        {"TEXTURE", {
          { "USE_MAP", OptVar{[](auto, auto, auto &gltf)->std::optional<std::monostate>{ 
            auto m = gltf.VrmMaterial();
            if(auto p=m.MainTexture())
            {
              return std::monostate{};
            }
            return std::nullopt;
          }}},
          { "USE_SHADEMULTIPLYTEXTURE", OptVar{[](auto, auto, auto &gltf)->std::optional<std::monostate>{
            auto m = gltf.VrmMaterial();
            if(auto p=m.ShadeTexture())
            {
              return std::monostate{};
            }
            return std::nullopt;
          }}},
          { "USE_MATCAPTEXTURE", OptVar{[](auto, auto, auto &gltf)->std::optional<std::monostate>{
            auto m = gltf.VrmMaterial();
            if(auto p=m.SphereAddTexture())
            {
              return std::monostate{};
            }
            return std::nullopt;
          }}},
        }},
        {"LIGHTING", {
          { "MTOON_USE_UV" },
          { "THREE_VRM_THREE_REVISION", ConstInt(150) },
          { "NUM_SPOT_LIGHT_COORDS", ConstInt(4) },
          { "NUM_DIR_LIGHTS", ConstInt(1) },
          { "NUM_POINT_LIGHTS", ConstInt(0) },
          { "NUM_SPOT_LIGHTS", ConstInt(0) },
          { "NUM_RECT_AREA_LIGHTS", ConstInt(0) },
          { "NUM_HEMI_LIGHTS", ConstInt(0) },
          { "NUM_SPOT_LIGHT_MAPS", ConstInt(0) },
          { "NUM_CLIPPING_PLANES", ConstInt(0) },
          { "UNION_CLIPPING_PLANES", ConstInt(0) },
          { "isOrthographic", ConstBool(false) },
        }},
        {"Material", {
          {"USE_ALPHATEST", OptVar{[](auto,auto,auto &gltf)->std::optional<std::monostate>{
            auto m = gltf.VrmMaterial();
            if(auto p = m.BlendMode())
            {
              if(*p == 1){
                return std::monostate{};
              }
            }
            return std::nullopt;
          }}},
        }},
      },
    },
    .UniformVarMap
    {
      //
      // Texture binding
      //

      { "map", ConstInt(0) },
      { "shadeMultiplyTexture", ConstInt(1) },
      { "normalMap", ConstInt(2) },
      { "shadingShiftTexture", ConstInt(3) },
      { "rimMultiplyTexture", ConstInt(4) },
      { "matcapTexture", ConstInt(5) },
      { "emissiveMap", ConstInt(6)},
      { "uvAnimationMaskTexture", ConstInt(7) },
    { "litFactor", RgbVar{ [](auto, auto, auto& gltf) {
      auto m = gltf.VrmMaterial();
      auto c = m.Color();
      return std::array<float, 3>{ c[0], c[1], c[2] };
      } } },
    { "shadeColorFactor", RgbVar{ [](auto, auto, auto& gltf) {
      auto m = gltf.VrmMaterial();
      return m.ShadeColor();
    } } },
    { "alphaTest", FloatVar{ [](auto, auto, auto& gltf) {
      auto m = gltf.VrmMaterial();
      if(auto p=m.Cutoff()){
        return *p;
      }
      return 0.5f;
    } } },
    //
    { "projectionMatrix",
      Mat4Var{ [](auto& w, auto& l, auto) { return w.ProjectionMatrix(); } } },
    { "viewMatrix",
      Mat4Var{ [](auto& w, auto& l, auto) { return w.ViewMatrix(); } } },
    { "cameraPosition",
      Vec3Var{ [](auto& w, auto& l, auto) { return w.CameraPosition(); } } },
    { "modelMatrix",
      Mat4Var{ [](auto& w, auto& l, auto) { return l.ModelMatrix; } } },
    { "normalMatrix",
      Mat3Var{ [](auto& w, auto& l, auto) { return l.NormalMatrix3; } } },
    { "modelViewMatrix", Mat4Var{ [](auto& w, auto& l, auto) {
        return mult(l.ModelMatrix, w.ViewMatrix());
      } } },
    { "uvTransform",
      Mat3Var{ [](auto& w, auto& l, auto) { return l.IdentityMatrix3; } } },
    { "mapUvTransform",
      Mat3Var{ [](auto& w, auto& l, auto) { return l.IdentityMatrix3; } } },
    { "opacity", ConstFloat(1) },
    { "ambientLightColor", Vec3Var{ [](auto, auto, auto) {
        return DirectX::XMFLOAT3{ 1, 1, 1 };
      } } },
    { "directionalLights[0].direction",
      Vec3Var{ [](auto, auto, auto) { return DirectX::XMFLOAT3(3, 3, 3); } } },
    { "directionalLights[0].color",
      Vec3Var{ [](auto, auto, auto) { return DirectX::XMFLOAT3(1, 1, 1); } } },
    }
    ,
    .UpdateState = [](auto, auto, auto &gltf)
    {
      auto m = gltf.VrmMaterial();

      // bool *ds;
      // if((ds = m.DoubleSided()) && *ds)
      // {
      //   glDisable(GL_CULL_FACE);
      //   glFrontFace(GL_CCW);
      // }
      // else{
      //   glEnable(GL_CULL_FACE);
      // }
    },
};

  auto m = gltfjson::vrm0::Material(mtoon0);
  if (auto p = m.MainTexture()) {
    if (auto texture =
          GetOrCreateTexture(root, bin, (uint32_t)*p, ColorSpace::Linear)) {
      ptr->Textures.push_back({ 0, texture });
    }
  }
  if (auto p = m.ShadeTexture()) {
    if (auto texture =
          GetOrCreateTexture(root, bin, (uint32_t)*p, ColorSpace::Linear)) {
      ptr->Textures.push_back({ 1, texture });
    }
  }
  if (auto p = m.SphereAddTexture()) {
    if (auto texture =
          GetOrCreateTexture(root, bin, (uint32_t)*p, ColorSpace::Linear)) {
      ptr->Textures.push_back({ 5, texture });
    }
  }

  return ptr;
}

} // namespace
