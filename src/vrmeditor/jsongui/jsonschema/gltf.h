#pragma once
#include "../json_widgets.h"
#include "json_prop.h"
#include <gltfjson/jsonpath.h>
#include <string>

namespace jsonschema {

inline std::list<gltfjson::JsonPathMap<JsonSchema>::KeyValue> Gltf = {
  {
    // https : //
    // github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/glTF.schema.json
    u8"/",
    { {
      { u8"asset", u8"📄", { {}, u8"{}" }, JsonPropFlags::Required },
      //
      { u8"extensions", u8"⭐", { {}, u8"{}" } },
      { u8"extensionsUsed", u8"⭐", { {}, u8"[]" } },
      { u8"extensionsRequired", u8"⭐", { {}, u8"[]" } },
      { u8"extras", u8"⭐", { {}, u8"{}" } },
      //
      { u8"buffers", u8"📦", { {}, u8"📦" } },
      { u8"bufferViews", u8"📦", { {}, u8"📦" } },
      { u8"accessors", u8"📦", { {}, u8"📦" } },
      //
      { u8"images", u8"🖼", { {}, u8"🖼" } },
      { u8"samplers", u8"🖼", { {}, u8"🖼" } },
      { u8"textures", u8"🖼", { {}, u8"🖼" } },
      { u8"materials", u8"💎", { {}, u8"💎" } },
      //
      { u8"meshes", u8"📐", { {}, u8"📐" } },
      { u8"skins", u8"📐", { {}, u8"📐" } },
      //
      { u8"nodes", u8"🛞", { {}, u8"🛞" } },
      { u8"scenes", u8"🛞", { {}, u8"🛞" } },
      { u8"scene", u8"🆔", { {}, u8"0" } },
    } },
  },
  {
    u8"/extensions",
    { {
      { u8"VRM", u8"🌟", { {}, u8"{}" } },
      { u8"VRMC_vrm", u8"🌟", { {}, u8"{}" } },
      { u8"VRMC_springBone", u8"🌟", { {}, u8"{}" } },
    } },
  },
  // VRM
  {
    // https : //
    // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.schema.json
    u8"/extensions/VRM",
    { {
      { u8"exporterVersion", u8"📄" },
      { u8"specVersion", u8"📄" },
      { u8"meta", u8"🪪" },
      { u8"humanoid", u8"👤" },
      { u8"firstPerson", u8"👀" },
      { u8"blendShapeMaster", u8"😀" },
      { u8"secondaryAnimation", u8"🔗" },
      { u8"materialProperties", u8"💎" },
    } },
  },
  {
    // https : //
    // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.meta.schema.json
    u8"/extensions/VRM/meta",
    { {
      { u8"title", u8"🪪" },
      { u8"version", u8"🪪" },
      { u8"author", u8"🪪" },
      { u8"contactInformation", u8"🪪" },
      { u8"reference", u8"🪪" },
      { u8"texture", u8"🆔" },
      { u8"allowedUserName", u8"🪪" },
      { u8"violentUssageName", u8"🪪" },
      { u8"sexualUssageName", u8"🪪" },
      { u8"commercialUssageName", u8"🪪" },
      { u8"otherPermissionUrl", u8"🪪" },
      { u8"licenseName", u8"🪪" },
      { u8"otherLicenseUrl", u8"🪪" },
    } },
  },
  {
    // https : //
    // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.humanoid.schema.json
    u8"/extensions/VRM/humanoid",
    { {
      { u8"humanBones", u8"🦴" },
      { u8"armStretch", u8"⛔" },
      { u8"legStretch", u8"⛔" },
      { u8"upperArmTwist", u8"⛔" },
      { u8"lowerArmTwist", u8"⛔" },
      { u8"upperLegTwist", u8"⛔" },
      { u8"lowerLegTwist", u8"⛔" },
      { u8"feetSpacing", u8"⛔" },
      { u8"hasTranslationDoF", u8"⛔" },
    } },
  },
  {
    // https : //
    // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.humanoid.bone.schema.json
    u8"/extensions/VRM/humanoid/humanBones/*",
    { {
      { u8"bone", u8"🦴" },
      { u8"node", u8"🆔" },
      { u8"useDefaultValues", u8"⛔" },
      { u8"min", u8"⛔" },
      { u8"max", u8"⛔" },
      { u8"center", u8"⛔" },
      { u8"axisLength", u8"⛔" },
    } },
  },
  {
    // https : //
    // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.firstperson.schema.json
    u8"/extensions/VRM/firstPerson",
    { {
      { u8"firstPersonBone", u8"🆔" },
      { u8"firstPersonBoneOffset", u8"↔" },
      { u8"meshAnnotations", u8"✨" },
      { u8"lookAtTypeName", u8"👀" },
      { u8"lookAtHorizontalInner", u8"👀" },
      { u8"lookAtHorizontalOuter", u8"👀" },
      { u8"lookAtVerticalDown", u8"👀" },
      { u8"lookAtVerticalUp", u8"👀" },
    } },
  },
  {
    // https : //
    // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.blendshape.schema.json
    u8"/extensions/VRM/blendShapeMaster",
    { {
      { u8"blendShapeGroups", u8"😀" },
    } },
  },
  {
    // https : //
    // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.blendshape.group.schema.json
    u8"/extensions/VRM/blendShapeMaster/blendShapeGroups/*",
    { {
      { u8"name", u8"📄" },
      { u8"presetName", u8"😀" },
      { u8"binds", u8"😀" },
      { u8"materialValues", u8"💎" },
      { u8"isBinary", u8"✅" },
    } },
  },
  {
    // https : //
    // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.secondaryanimation.schema.json
    u8"/extensions/VRM/secondaryanimation",
    { {
      { u8"boneGroups", u8"🔗" },
      { u8"colliderGroups", u8"🎱" },
    } },
  },
  {
    // https : //
    // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.material.schema.json
    u8"/extensions/VRM/materialProperties/*",
    { {
      { u8"name", u8"📄" },
      { u8"shader", u8"📄" },
      { u8"renderQueue", u8"🔢" },
      { u8"floatProperties", u8"🔢" },
      { u8"vectorProperties", u8"🔢" },
      { u8"textureProperties", u8"🖼" },
      { u8"keywordMap", u8"📄" },
      { u8"tagMap", u8"✅" },
    } },
  },
  // VRMC_vrm
  {
    // https : //
    // github.com/vrm-c/vrm-specification/blob/master/specification/VRMC_vrm-1.0/schema/VRMC_vrm.schema.json
    u8"/extensions/VRMC_vrm",
    { {
      { u8"specVersion", u8"📄" },
      { u8"meta", u8"🪪" },
      { u8"humanoid", u8"👤" },
      { u8"firstPerson", u8"✨" },
      { u8"lookAt", u8"👀" },
      { u8"expressions", u8"😀" },
    } },
  },
  {
    // https://github.com/vrm-c/vrm-specification/blob/master/specification/VRMC_vrm-1.0/schema/VRMC_vrm.expressions.schema.json
    u8"/extensions/VRMC_vrm/expressions/preset",
    { {
      { u8"happy", u8"😆" },
      { u8"angry", u8"😠" },
      { u8"sad", u8"😥" },
      { u8"relaxed", u8"🙂" },
      { u8"surprised", u8"😲" },
      { u8"aa", u8"👄" },
      { u8"ih", u8"👄" },
      { u8"ou", u8"👄" },
      { u8"ee", u8"👄" },
      { u8"oh", u8"👄" },
      { u8"blink", u8"😉" },
      { u8"blinkLeft", u8"😉" },
      { u8"blinkRight", u8"😉" },
      { u8"lookUp", u8"👀" },
      { u8"lookDown", u8"👀" },
      { u8"lookLeft", u8"👀" },
      { u8"lookRight", u8"👀" },
      { u8"neutral", u8"😶" },
    } },
  },
  // glTF
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
      {
        u8"byteLength",
        u8"🔢",
        {},
        JsonPropFlags::Required,
      },
      {
        u8"uri",
        u8"📄",
        {},
      },
    } },
  },
  {
    // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/bufferView.schema.json
    u8"/bufferViews/*",
    { {
      { u8"name", u8"📄", { {}, U8Q("") } },
      { u8"buffer", u8"🆔", {}, JsonPropFlags::Required },
      { u8"byteLength", u8"📄", {}, JsonPropFlags::Required },
    } },
  },
  {
    // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/accessor.schema.json
    u8"/accessors/*",
    { {
      { u8"name", u8"📄", { {}, U8Q("") } },
      { u8"componentType", u8"🔢", {}, JsonPropFlags::Required },
      { u8"type", u8"📄", {}, JsonPropFlags::Required },
      { u8"count", u8"🔢", {}, JsonPropFlags::Required },
    } },
  },
  // image/sampler/texture/material
  {
    // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/image.schema.json
    u8"/images/*",
    { {
      { u8"name", u8"📄", { {}, U8Q("") } },
      { u8"uri", u8"📄" },
      { u8"mimeType", u8"📄" },
      { u8"bufferView", u8"🆔" },
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
      { u8"extensions", u8"⭐", { {}, u8"{}" } },
      { u8"pbrMetallicRoughness", u8"💎", { {}, u8"{}" } },
      { u8"normalTexture", u8"🖼", { {}, u8"{}" } },
      { u8"occlusionTexture", u8"🖼", { {}, u8"{}" } },
      { u8"emissiveTexture", u8"🖼", { {}, u8"{}" } },
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
      { u8"KHR_materials_unlit", u8"🏛️", { {}, u8"{}" } },
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
      { u8"metallicRoughnessTexture", u8"🖼", { {}, u8"{}" } },
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
      { u8"scale", u8"🔢", { {}, u8"{1,1,1}", JsonValueFlags::DefaultIfNone } },
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

} // namespace
