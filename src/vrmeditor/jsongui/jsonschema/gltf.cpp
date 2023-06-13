#include "gltf.h"
#include "../json_widgets.h"

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
        { u8"asset", u8"📄", JsonValue::Object, JsonPropFlags::Required },
        //
        { u8"extensions", u8"⭐", JsonValue::Object },
        { u8"extensionsUsed", u8"⭐", JsonValue::Array },
        { u8"extensionsRequired", u8"⭐", JsonValue::Array },
        { u8"extras", u8"⭐", JsonValue::Object },
        //
        { u8"buffers", u8"🫙", JsonValue::Array },
        { u8"bufferViews", u8"🫙", JsonValue::Array },
        { u8"accessors", u8"🫙", JsonValue::Array },
        //
        { u8"images", u8"🖼", JsonValue::Array },
        { u8"samplers", u8"🖼", JsonValue::Array },
        { u8"textures", u8"🖼", JsonValue::Array },
        { u8"materials", u8"💎", JsonValue::Array },
        //
        { u8"meshes", u8"📐", JsonValue::Array },
        { u8"skins", u8"📐", JsonValue::Array },
        //
        { u8"nodes", u8"🛞", JsonValue::Array },
        { u8"scenes", u8"🛞", JsonValue::Array },
        { u8"scene", u8"🆔", JsonValue::Number },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/asset.schema.json
      u8"/asset",
      { {
        { u8"version", u8"📄", { {}, U8Q("") }, JsonPropFlags::Required },
        { u8"minVersion", u8"📄", { {}, U8Q("") } },
        { u8"copyright", u8"📄", { {}, U8Q("") } },
        { u8"generator", u8"📄", { {}, U8Q("") } },
      } },
    },
    // buffer/bufferView/accessor
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/buffer.schema.json
      u8"/buffers/*",
      { {
        { u8"name", u8"📄", { {}, U8Q("") } },
        { u8"uri", u8"📄", { {}, U8Q("") } },
        { u8"byteLength", u8"🔢", { {}, u8"0" }, JsonPropFlags::Required },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/bufferView.schema.json
      u8"/bufferViews/*",
      { {
        { u8"name", u8"📄", { {}, U8Q("") } },
        { u8"buffer", u8"🆔", { {}, u8"0" }, JsonPropFlags::Required },
        { u8"byteLength", u8"🔢", { {}, u8"0" }, JsonPropFlags::Required },
        { u8"byteOffset", u8"🔢", { {}, u8"0" } },
        { u8"byteStride", u8"🔢", { {}, u8"0" } },
        { u8"target", u8"🔢", { {}, u8"0" } },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/accessor.schema.json
      u8"/accessors/*",
      { {
        { u8"name", u8"📄", { {}, U8Q("") } },
        { u8"bufferView", u8"🆔", { {}, u8"0" } },
        { u8"byteOffset", u8"🔢", { {}, u8"0" } },
        { u8"componentType", u8"🔢", {}, JsonPropFlags::Required },
        { u8"type", u8"📄", {}, JsonPropFlags::Required },
        { u8"count", u8"🔢", {}, JsonPropFlags::Required },
        { u8"normalized", u8"✅", { {}, u8"false" } },
        { u8"max", u8"🔢", JsonValue::Array },
        { u8"min", u8"🔢", JsonValue::Array },
        { u8"sparse", u8"🫙", JsonValue::Object },
      } },
    },
    // image/sampler/texture/material
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/image.schema.json
      u8"/images/*",
      { {
        { u8"name", u8"📄", { {}, U8Q("") } },
        { u8"uri", u8"📄", { {}, U8Q("") } },
        { u8"mimeType", u8"📄", { {}, U8Q("") } },
        { u8"bufferView", u8"🆔", { {}, u8"0" } },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/sampler.schema.json
      u8"/samplers/*",
      { {
        { u8"name", u8"📄", { {}, U8Q("") } },
        { u8"magFilter", u8"🔢" },
        { u8"minFilter", u8"🔢" },
        { u8"wrapS", u8"🔢" },
        { u8"wrapT", u8"🔢" },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/texture.schema.json
      u8"/textures/*",
      { {
        { u8"name", u8"📄", { {}, U8Q("") } },
        { u8"source", u8"🆔" },
        { u8"sampler", u8"🆔" },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/material.schema.json
      u8"/materials/*",
      { {
        { u8"name", u8"📄", { {}, U8Q("") } },
        { u8"extensions", u8"⭐", JsonValue::Object },
        { u8"pbrMetallicRoughness", u8"💎", JsonValue::Object },
        { u8"normalTexture", u8"🖼", JsonValue::Object },
        { u8"occlusionTexture", u8"🖼", JsonValue::Object },
        { u8"emissiveTexture", u8"🖼", JsonValue::Object },
        { u8"emissiveFactor", u8"🎨", { RgbPicker{}, u8"[0,0,0]" } },
        { u8"alphaMode",
          u8"👻",
          { StringEnum{ { "OPAQUE", "MASK", "BLEND" } }, U8Q("OPAQUE") } },
        { u8"alphaCutoff", u8"👻", { FloatSlider{}, u8"0.5" } },
        { u8"doubleSided", u8"✅", { {}, u8"false" } },
      } },
    },
    {
      u8"/materials/*/extensions",
      { {
        { u8"KHR_materials_unlit", u8"🏛️", JsonValue::Object },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/material.pbrMetallicRoughness.schema.json
      u8"/materials/*/pbrMetallicRoughness",
      { {
        { u8"baseColorFactor", u8"🎨", { RgbaPicker{} } },
        { u8"baseColorTexture", u8"🖼" },
        {
          u8"metallicFactor",
          u8"🎚️",
          { FloatSlider{}, u8"1" },
        },
        {
          u8"roughnessFactor",
          u8"🎚️",
          { FloatSlider{}, u8"1" },
        },
        { u8"metallicRoughnessTexture", u8"🖼", JsonValue::Object },
      } },
    },
    // mesh/skin
    {
      // https
      // ://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/mesh.primitive.schema.json
      u8"/meshes/*/primitives/*",
      { {
        { u8"attributes", u8"📄", {}, JsonPropFlags::Required },
        { u8"indices", u8"📄", {} },
        { u8"material", u8"🆔" },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/mesh.schema.json
      u8"/meshes/*",
      { {
        { u8"name", u8"📄", { {}, U8Q("") } },
        { u8"primitives", u8"📐", {}, JsonPropFlags::Required },
        { u8"weights", u8"🔢" },
      } },
    },
    // node
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/node.schema.json
      u8"/nodes/*",
      { {
        { u8"name", u8"📄", { {}, U8Q("") } },
        { u8"mesh", u8"🆔", { SelectMesh, u8"0" } },
        { u8"children", u8"🆔" },
        { u8"translation",
          u8"🔢",
          { {}, u8"{0,0,0}", JsonValueFlags::DefaultIfNone } },
        { u8"rotation",
          u8"🔢",
          { {}, u8"{0,0,0,1}", JsonValueFlags::DefaultIfNone } },
        { u8"scale",
          u8"🔢",
          { {}, u8"{1,1,1}", JsonValueFlags::DefaultIfNone } },
        { u8"matrix",
          u8"🔢",
          { {},
            u8"{1,0,0,0"
            u8",0,1,0,0"
            u8",0,0,1,0"
            u8",0,0,0,1}" } },
      } },
    },
    {
      // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/skin.schema.json
      u8"/skins/*",
      { {
        { u8"name", u8"📄", { {}, U8Q("") } },
      } },
    },
  };
}

} // namespace
