#include "gltf.h"
#include "../json_widgets.h"
#include "tag.h"

namespace jsonschema {

std::list<gltfjson::JsonPathMap<JsonSchema>::KeyValue>
Gltf()
{
  return {
    {
      // https : //
      // github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/glTF.schema.json
      u8"/",
      { {
        { { u8"📄", u8"asset" }, JsonValue::Object, JsonPropFlags::Required },
        //
        { { u8"⭐", u8"extensions" }, JsonValue::Object, JsonPropFlags::NoRemove },
        { { u8"⭐", u8"extensionsUsed" }, JsonValue::Array, JsonPropFlags::NoRemove },
        { { u8"⭐", u8"extensionsRequired" }, JsonValue::Array, JsonPropFlags::NoRemove },
        { { u8"⭐", u8"extras" }, JsonValue::Object, JsonPropFlags::NoRemove },
        //
        { { u8"🫙", u8"buffers" }, JsonValue::Array, JsonPropFlags::NoRemove },
        { { u8"🫙", u8"bufferViews" }, JsonValue::Array, JsonPropFlags::NoRemove },
        { { u8"🫙", u8"accessors" }, JsonValue::Array, JsonPropFlags::NoRemove },
        //
        { { u8"🖼", u8"images" }, JsonValue::Array, JsonPropFlags::NoRemove },
        { { u8"🖼", u8"samplers" }, JsonValue::Array, JsonPropFlags::NoRemove },
        { { u8"🖼", u8"textures" }, JsonValue::Array, JsonPropFlags::NoRemove },
        { { u8"💎", u8"materials" }, JsonValue::Array, JsonPropFlags::NoRemove },
        //
        { { u8"📐", u8"meshes" }, JsonValue::Array, JsonPropFlags::NoRemove },
        { { u8"📐", u8"skins" }, JsonValue::Array, JsonPropFlags::NoRemove },
        //
        { { u8"🛞", u8"nodes" }, JsonValue::Array, JsonPropFlags::NoRemove },
        { { u8"🛞", u8"scenes" }, JsonValue::Array, JsonPropFlags::NoRemove },
        { { u8"🆔", u8"scene" }, JsonValue::Number },
        //
        { { u8"🎬", u8"animations" }, JsonValue::Array, JsonPropFlags::NoRemove },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/asset.schema.json
      u8"/asset",
      { {
        { { u8"📄", u8"version" }, JsonValue::String, JsonPropFlags::Required },
        { { u8"📄", u8"minVersion" }, JsonValue::String },
        { { u8"📄", u8"copyright" }, JsonValue::String },
        { { u8"📄", u8"generator" }, JsonValue::String },
      } },
    },
    // buffer/bufferView/accessor
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/buffer.schema.json
      u8"/buffers/*",
      { {
        { { u8"📄", u8"name" }, JsonValue::String },
        { { u8"📄", u8"uri" }, JsonValue::String },
        { { u8"🔢", u8"byteLength" },
          JsonValue::Number,
          JsonPropFlags::Required },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/bufferView.schema.json
      u8"/bufferViews/*",
      { {
        { { u8"📄", u8"name" }, JsonValue::String },
        { { u8"🆔", u8"buffer" }, JsonValue::Number, JsonPropFlags::Required },
        { { u8"🔢", u8"byteLength" },
          JsonValue::Number,
          JsonPropFlags::Required },
        { { u8"🔢", u8"byteOffset" }, JsonValue::Number },
        { { u8"🔢", u8"byteStride" }, JsonValue::Number },
        { { u8"🔢", u8"target" }, JsonValue::Number },
      } },
    },
    {
      u8"/accessors",
      { {
        { { u8"🫙" }, JsonValue::Object, {}, AccessorTag },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/accessor.schema.json
      u8"/accessors/*",
      { {
        { { u8"📄", u8"name" }, JsonValue::String },
        { { u8"🆔", u8"bufferView" }, JsonValue::Number },
        { { u8"🔢", u8"byteOffset" }, JsonValue::Number },
        { { u8"🔢", u8"componentType" }, {}, JsonPropFlags::Required },
        { { u8"📄", u8"type" }, {}, JsonPropFlags::Required },
        { { u8"🔢", u8"count" }, {}, JsonPropFlags::Required },
        { { u8"✅", u8"normalized" }, { {}, u8"false" } },
        { { u8"🔢", u8"max" }, JsonValue::Array },
        { { u8"🔢", u8"min" }, JsonValue::Array },
        { { u8"🫙", u8"sparse" }, JsonValue::Object },
      } },
    },
    // image/sampler/texture/material
    {
      u8"/images",
      { {
        { { u8"🖼" }, JsonValue::Object, {}, ImageTag },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/image.schema.json
      u8"/images/*",
      { {
        { { u8"📄", u8"name" }, JsonValue::String },
        { { u8"📄", u8"uri" }, JsonValue::String },
        { { u8"📄", u8"mimeType" }, JsonValue::String },
        { { u8"🆔", u8"bufferView" }, JsonValue::Number },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/sampler.schema.json
      u8"/samplers/*",
      { {
        { { u8"📄", u8"name" }, JsonValue::String },
        { { u8"🔢", u8"magFilter" } },
        { { u8"🔢", u8"minFilter" } },
        { { u8"🔢", u8"wrapS" } },
        { { u8"🔢", u8"wrapT" } },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/texture.schema.json
      u8"/textures/*",
      { {
        { { u8"📄", u8"name" }, JsonValue::String },
        { { u8"🆔", u8"source" } },
        { { u8"🆔", u8"sampler" } },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/material.schema.json
      u8"/materials",
      { {
        { { u8"💎" }, JsonValue::Object, {}, MaterialTag },
      } },
    },
    {
      u8"/materials/*",
      { {
        { { u8"📄", u8"name" }, JsonValue::String },
        { { u8"⭐", u8"extensions" }, JsonValue::Object },
        { { u8"💎", u8"pbrMetallicRoughness" }, JsonValue::Object },
        { { u8"🖼", u8"normalTexture" }, JsonValue::Object },
        { { u8"🖼", u8"occlusionTexture" }, JsonValue::Object },
        { { u8"🖼", u8"emissiveTexture" }, JsonValue::Object },
        { { u8"🎨", u8"emissiveFactor" }, { RgbPicker{}, u8"[0,0,0]" } },
        { { u8"👻", u8"alphaMode" },
          { StringEnum{ { "OPAQUE", "MASK", "BLEND" } }, U8Q("OPAQUE") } },
        { { u8"👻", u8"alphaCutoff" }, { FloatSlider{}, u8"0.5" } },
        { { u8"✅", u8"doubleSided" }, { {}, u8"false" } },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/material.pbrMetallicRoughness.schema.json
      u8"/materials/*/pbrMetallicRoughness",
      { {
        { { u8"🎨", u8"baseColorFactor" }, { RgbaPicker{} } },
        { { u8"🖼", u8"baseColorTexture" } },
        {
          { u8"🎚️", u8"metallicFactor" },
          { FloatSlider{}, u8"1" },
        },
        {
          { u8"🎚️", u8"roughnessFactor" },
          { FloatSlider{}, u8"1" },
        },
        { { u8"🖼", u8"metallicRoughnessTexture" }, JsonValue::Object },
      } },
    },
    // mesh/skin
    {
      // https
      // ://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/mesh.primitive.schema.json
      u8"/meshes/*/primitives/*",
      { {
        { { u8"📄", u8"attributes" }, {}, JsonPropFlags::Required },
        { { u8"📄", u8"indices" }, {} },
        { { u8"🆔", u8"material" } },
      } },
    },
    {
      u8"/meshes",
      { {
        { { u8"📐" }, JsonValue::Object, {}, MeshTag },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/mesh.schema.json
      u8"/meshes/*",
      { {
        { { u8"📄", u8"name" }, JsonValue::String },
        { { u8"📐", u8"primitives" }, {}, JsonPropFlags::Required },
        { { u8"🔢", u8"weights" } },
      } },
    },
    // node
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/node.schema.json
      u8"/nodes",
      { {
        { { u8"🛞", u8"" }, JsonValue::Object, {}, NodeTag },
      } },
    },
    {
      u8"/nodes/*",
      { {
        { { u8"📄", u8"name" }, JsonValue::String },
        { { u8"🆔", u8"mesh" }, { SelectMesh, u8"0" } },
        { { u8"🆔", u8"children" } },
        { { u8"🔢", u8"translation" }, { {}, u8"{0,0,0}" } },
        { { u8"🔢", u8"rotation" }, { {}, u8"{0,0,0,1}" } },
        { { u8"🔢", u8"scale" }, { {}, u8"{1,1,1}" } },
        { { u8"🔢", u8"matrix" },
          { {},
            u8"{1,0,0,0"
            u8",0,1,0,0"
            u8",0,0,1,0"
            u8",0,0,0,1}" } },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/scene.schema.json
      u8"/scenes",
      { {
        {
          { u8"🛞" },
          { {}, u8"{}" },
          {},
        },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/skin.schema.json
      u8"/skins/*",
      { {
        { { u8"📄", u8"name" }, JsonValue::String },
      } },
    },
  };
}

} // namespace
