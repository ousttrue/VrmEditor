#pragma once
#include "extensions.h"
#include "../json_widgets.h"

namespace jsonschema {

std::list<gltfjson::JsonPathMap<JsonSchema>::KeyValue>
Extensions()
{
  // https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_texture_transform/schema/KHR_texture_transform.textureInfo.schema.json
  JsonSchema KHR_texture_transform{ {
    { { u8"🔢", u8"offset" }, JsonValue::Array },
    { { u8"🔢", u8"rotation" }, JsonValue::Number },
    { { u8"🔢", u8"scale" }, JsonValue::Array },
    { { u8"🆔", u8"texCoord" }, JsonValue::Number },
  } };

  return {
    {
      u8"/extensions",
      { {
        { { u8"🌟", u8"VRM" }, JsonValue::Object },
        { { u8"🌟", u8"VRMC_vrm" }, JsonValue::Object },
        { { u8"🌟", u8"VRMC_springBone" }, JsonValue::Object },
        { { u8"🌟", u8"VRMC_vrm_animation" }, JsonValue::Object },
        { { u8"🗿", u8"KHR_lights_punctual" }, JsonValue::Object },
      } },
    },
    {
      u8"/materials/*/extensions",
      { {
        { { u8"🗿", u8"KHR_materials_unlit" }, JsonValue::Object },
        { { u8"🗿", u8"KHR_materials_emissive_strength" }, JsonValue::Object },
      } },
    },
    {
      u8"/materials/*/pbrMetallicRoughness/baseColorTexture/extensions",
      { {
        { { u8"🗿", u8"KHR_texture_transform" }, JsonValue::Object },
      } },
    },
    {
      u8"/materials/*/pbrMetallicRoughness/metallicRoughnessTexture/extensions",
      { {
        { { u8"🗿", u8"KHR_texture_transform" }, JsonValue::Object },
      } },
    },
    {
      u8"/materials/*/normalTexture/extensions",
      { {
        { { u8"🗿", u8"KHR_texture_transform" }, JsonValue::Object },
      } },
    },
    {
      u8"/materials/*/occlusionTexture/extensions",
      { {
        { { u8"🗿", u8"KHR_texture_transform" }, JsonValue::Object },
      } },
    },
    {
      u8"/materials/*/emissiveTexture/extensions",
      { {
        { { u8"🗿", u8"KHR_texture_transform" }, JsonValue::Object },
      } },
    },
    {
      u8"/nodes/*/extensions",
      { {
        { { u8"🗿", u8"KHR_lights_punctual" }, JsonValue::Object },
      } },
    },
    //
    //
    //
    {
      // https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_lights_punctual/schema/light.schema.json
      u8"/extensions/KHR_lights_punctual",
      { { { { u8"💡", u8"lights" }, JsonValue::Array } } },
    },
    {
      u8"/extensions/KHR_lights_punctual/lights",
      { {
        { { u8"💡" }, JsonValue::Object },
      } },
    },
    {
      u8"/extensions/KHR_lights_punctual/lights/*",
      { {
        { { u8"📄", u8"name" }, JsonValue::String },
        { { u8"🎨", u8"color" }, { RgbPicker{}, u8"[1,1,1]" } },
        { { u8"🔢", u8"intensity" }, { FloatSlider{}, u8"1" } },
        { { u8"💡", u8"spot" }, JsonValue::Object },
        { { u8"📄", u8"type" },
          { StringEnum{ { "directional", "spot", "point" } } } },
        { { u8"🔢", u8"range" }, JsonValue::Number },
      } },
    },
    {
      u8"/nodes/*/extensions/KHR_lights_punctual",
      { {
        { { u8"🆔", u8"light" }, JsonValue::Number },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_unlit/schema/glTF.KHR_materials_unlit.schema.json
      u8"/materials/*/extensions/KHR_materials_unlit",
      { {} },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_emissive_strength/schema/glTF.KHR_materials_emissive_strength.schema.json
      u8"/materials/*/extensions/KHR_materials_emissive_strength",
      { {
        { { u8"🔢", u8"emissiveStrength" }, { {}, u8"1" } },
      } },
    },
    // TextureInfo
    {
      u8"/materials/*/pbrMetallicRoughness/baseColorTexture/extensions/"
      u8"KHR_texture_transform",
      KHR_texture_transform,
    },
    {
      u8"/materials/*/pbrMetallicRoughness/metallicRoughnessTexture/extensions/"
      u8"KHR_texture_transform",
      KHR_texture_transform,
    },
    {
      u8"/materials/*/normalTexture/extensions/"
      u8"KHR_texture_transform",
      KHR_texture_transform,
    },
    {
      u8"/materials/*/occlusionTexture/extensions/"
      u8"KHR_texture_transform",
      KHR_texture_transform,
    },
    {
      u8"/materials/*/emissiveStrength/extensions/"
      u8"KHR_texture_transform",
      KHR_texture_transform,
    },
  };
}

} // namespace
