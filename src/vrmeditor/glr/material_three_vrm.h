#pragma once
#include "material_factory.h"
#include "shader_source.h"
#include <gltfjson.h>
#include <grapho/gl3/material.h>
#include <string_view>
#include <vector>

namespace glr {

const auto VS_CODE = u8R"(
// = object.matrixWorld
uniform mat4 modelMatrix;

// = camera.matrixWorldInverse * object.matrixWorld
uniform mat4 modelViewMatrix;

// = camera.projectionMatrix
uniform mat4 projectionMatrix;

// = camera.matrixWorldInverse
uniform mat4 viewMatrix;

// = inverse transpose of modelViewMatrix
uniform mat3 normalMatrix;

// = camera position in world space
uniform vec3 cameraPosition;

// default vertex attributes provided by BufferGeometry
attribute vec3 position;
attribute vec3 normal;
attribute vec2 uv;

#ifdef USE_TANGENT
	attribute vec4 tangent;
#endif
#if defined( USE_COLOR_ALPHA )
	// vertex color attribute with alpha
	attribute vec4 color;
#elif defined( USE_COLOR )
	// vertex color attribute
	attribute vec3 color;
#endif

#ifdef USE_MORPHTARGETS

	attribute vec3 morphTarget0;
	attribute vec3 morphTarget1;
	attribute vec3 morphTarget2;
	attribute vec3 morphTarget3;

	#ifdef USE_MORPHNORMALS

		attribute vec3 morphNormal0;
		attribute vec3 morphNormal1;
		attribute vec3 morphNormal2;
		attribute vec3 morphNormal3;

	#else

		attribute vec3 morphTarget4;
		attribute vec3 morphTarget5;
		attribute vec3 morphTarget6;
		attribute vec3 morphTarget7;

	#endif
#endif
#ifdef USE_SKINNING
	attribute vec4 skinIndex;
	attribute vec4 skinWeight;
#endif
#ifdef USE_INSTANCING
	// Note that modelViewMatrix is not set when rendering an instanced model,
	// but can be calculated from viewMatrix * modelMatrix.
	//
	// Basic Usage:
	//   gl_Position = projectionMatrix * viewMatrix * modelMatrix * instanceMatrix * vec4(position, 1.0);
	attribute mat4 instanceMatrix;
#endif
)";

const auto FS_CODE = u8R"(
uniform mat4 viewMatrix;
uniform vec3 cameraPosition;

vec4 LinearToLinear( in vec4 value ) {
    return value;
}
vec4 GammaToLinear( in vec4 value, in float gammaFactor ) {
    return vec4( pow( value.xyz, vec3( gammaFactor ) ), value.w );
}
vec4 LinearToGamma( in vec4 value, in float gammaFactor ) {
    return vec4( pow( value.xyz, vec3( 1.0 / gammaFactor ) ), value.w );
}
vec4 sRGBToLinear( in vec4 value ) {
    return vec4( mix( pow( value.rgb * 0.9478672986 + vec3( 0.0521327014 ), vec3( 2.4 ) ), value.rgb * 0.0773993808, vec3( lessThanEqual( value.rgb, vec3( 0.04045 ) ) ) ), value.w );
}
vec4 LinearTosRGB( in vec4 value ) {
    return vec4( mix( pow( value.rgb, vec3( 0.41666 ) ) * 1.055 - vec3( 0.055 ), value.rgb * 12.92, vec3( lessThanEqual( value.rgb, vec3( 0.0031308 ) ) ) ), value.w );
}
vec4 RGBEToLinear( in vec4 value ) {
    return vec4( value.rgb * exp2( value.a * 255.0 - 128.0 ), 1.0 );
}
vec4 LinearToRGBE( in vec4 value ) {
    float maxComponent = max( max( value.r, value.g ), value.b );
    float fExp = clamp( ceil( log2( maxComponent ) ), -128.0, 127.0 );
    return vec4( value.rgb / exp2( fExp ), ( fExp + 128.0 ) / 255.0 );
}
vec4 RGBMToLinear( in vec4 value, in float maxRange ) {
    return vec4( value.xyz * value.w * maxRange, 1.0 );
}
vec4 LinearToRGBM( in vec4 value, in float maxRange ) {
    float maxRGB = max( value.x, max( value.g, value.b ) );
    float M      = clamp( maxRGB / maxRange, 0.0, 1.0 );
    M            = ceil( M * 255.0 ) / 255.0;
    return vec4( value.rgb / ( M * maxRange ), M );
}
vec4 RGBDToLinear( in vec4 value, in float maxRange ) {
    return vec4( value.rgb * ( ( maxRange / 255.0 ) / value.a ), 1.0 );
}
vec4 LinearToRGBD( in vec4 value, in float maxRange ) {
    float maxRGB = max( value.x, max( value.g, value.b ) );
    float D      = max( maxRange / maxRGB, 1.0 );
    D            = min( floor( D ) / 255.0, 1.0 );
    return vec4( value.rgb * ( D * ( 255.0 / maxRange ) ), D );
}
const mat3 cLogLuvM = mat3( 0.2209, 0.3390, 0.4184, 0.1138, 0.6780, 0.7319, 0.0102, 0.1130, 0.2969 );
vec4 LinearToLogLuv( in vec4 value ) {
    vec3 Xp_Y_XYZp = value.rgb * cLogLuvM;
    Xp_Y_XYZp = max(Xp_Y_XYZp, vec3(1e-6, 1e-6, 1e-6));
    vec4 vResult;
    vResult.xy = Xp_Y_XYZp.xy / Xp_Y_XYZp.z;
    float Le = 2.0 * log2(Xp_Y_XYZp.y) + 127.0;
    vResult.w = fract(Le);
    vResult.z = (Le - (floor(vResult.w*255.0))/255.0)/255.0;
    return vResult;
}
const mat3 cLogLuvInverseM = mat3( 6.0014, -2.7008, -1.7996, -1.3320, 3.1029, -5.7721, 0.3008, -1.0882, 5.6268 );
vec4 LogLuvToLinear( in vec4 value ) {
    float Le = value.z * 255.0 + value.w;
    vec3 Xp_Y_XYZp;
    Xp_Y_XYZp.y = exp2((Le - 127.0) / 2.0);
    Xp_Y_XYZp.z = Xp_Y_XYZp.y / value.y;
    Xp_Y_XYZp.x = value.x * Xp_Y_XYZp.z;
    vec3 vRGB = Xp_Y_XYZp.rgb * cLogLuvInverseM;
    return vec4( max(vRGB, 0.0), 1.0 );
}

vec4 mapTexelToLinear( vec4 value ) { return LinearToLinear( value ); }
vec4 envMapTexelToLinear( vec4 value ) { return LinearToLinear( value ); }
vec4 emissiveMapTexelToLinear( vec4 value ) { return LinearToLinear( value ); }
vec4 linearToOutputTexel( vec4 value ) { return LinearToLinear( value ); }
  )";

inline DirectX::XMFLOAT4X4
mult(const DirectX::XMFLOAT4X4& lhs, const DirectX::XMFLOAT4X4& rhs)
{
  DirectX::XMFLOAT4X4 m;
  DirectX::XMStoreFloat4x4(
    &m, DirectX::XMLoadFloat4x4(&lhs) * DirectX::XMLoadFloat4x4(&rhs));
  return m;
}

inline std::shared_ptr<MaterialFactory>
MaterialFactory_MToon(const gltfjson::typing::Root& root,
                      const gltfjson::typing::Bin& bin,
                      std::optional<uint32_t> materialId)
{
  auto ptr = std::make_shared<MaterialFactory>();

  *ptr = MaterialFactory{
    .Type = ShaderTypes::MToon0,
    .VS={
      .SourceName = "mtoon.vert",
      .Version = u8"#version 300 es",
      .Codes{ VS_CODE },
      .Macros = {
        { u8"THREE_VRM_THREE_REVISION", 150 },
        { u8"NUM_SPOT_LIGHT_COORDS", 4 },
        { u8"NUM_CLIPPING_PLANES", 0 },
        { u8"MTOON_USE_UV" },
      },
    },
    .FS={
      .SourceName = "mtoon.frag",
      .Version =u8"#version 300 es", 
      .Precision = u8"mediump float",
      .Codes{ FS_CODE },
      .Macros = {
        // { u8"DEBUG_LITSHADERATE" },
        // { u8"DEBUG_UV" },
        // { u8"DEBUG_NORMAL" },
        { u8"USE_MAP" },
        { u8"MTOON_USE_UV" },
        { u8"THREE_VRM_THREE_REVISION", 150 },
        { u8"NUM_SPOT_LIGHT_COORDS", 4 },
        { u8"NUM_DIR_LIGHTS", 2 },
        { u8"NUM_POINT_LIGHTS", 0 },
        { u8"NUM_SPOT_LIGHTS", 0 },
        { u8"NUM_RECT_AREA_LIGHTS", 0 },
        { u8"NUM_HEMI_LIGHTS", 0 },
        { u8"NUM_SPOT_LIGHT_MAPS", 0 },
        { u8"NUM_CLIPPING_PLANES", 0 },
        { u8"UNION_CLIPPING_PLANES", 0 },
        { u8"isOrthographic", false },
      },
    },
    .UniformBinds
    {
      {"projectionMatrix",[](auto &w,auto &l){return w.ProjectionMatrix();}},
      {"viewMatrix",[](auto &w,auto &l){return w.ViewMatrix();}},
      {"cameraPosition",[](auto &w,auto &l){return w.CameraPosition();}},
      {"modelMatrix",[](auto &w,auto &l){return l.ModelMatrix();}},
      {"normalMatrix",[](auto &w,auto &l){return l.NormalMatrix3();}},
      {"modelViewMatrix",[](auto &w,auto &l){return mult(l.ModelMatrix(), w.ViewMatrix());}},
      {"uvTransform",[](auto &w,auto &l){return l.UvTransformMatrix();}},
      {"map",GetInt(0)},
      {"opacity",GetFloat(1)},
      {"litFactor",[](auto, auto){return DirectX::XMFLOAT3{1,1,1};}},

      {"directionalLights[0].direction", [](auto, auto){ return DirectX::XMFLOAT3(3,3,3);}},
      {"directionalLights[0].color", [](auto, auto){ return DirectX::XMFLOAT3(1,1,1);}},
    },
  };

  if (materialId) {
    auto src = root.Materials[*materialId];
    if (auto extensions = src.Extensions()) {

      gltfjson::tree::NodePtr mtoon1;
      if (extensions) {
        if (auto mtoon1 = extensions->Get(u8"VRMC_materials_mtoon")) {
          ptr->Type = ShaderTypes::MToon1;
        }
      }
    }

    if (auto root_extensins = root.Extensions()) {
      if (auto VRM = root_extensins->Get(u8"VRM")) {
        if (auto props = VRM->Get(u8"materialProperties")) {
          if (auto array = props->Array()) {
            if (*materialId < array->size()) {
              auto mtoonMaterial = (*array)[*materialId];
              if (auto textures = mtoonMaterial->Get(u8"textureProperties")) {
                if (auto obj = textures->Object()) {
                  for (auto kv : *obj) {
                    if (kv.first == u8"_MainTex") {
                      if (auto pValue = kv.second->Ptr<float>()) {
                        if (auto texture = GetOrCreateTexture(
                              root, bin, (uint32_t)*pValue, ColorSpace::sRGB)) {
                          ptr->Textures.push_back({ 0, texture });
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  return ptr;
}

}
