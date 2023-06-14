#pragma once
#include "vrm0.h"
#include "../json_widgets.h"

namespace jsonschema {

std::list<gltfjson::JsonPathMap<JsonSchema>::KeyValue>
VRM()
{
  return {
    {
      // https : //
      // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.schema.json
      u8"/extensions/VRM",
      { {
        { { u8"📄", u8"exporterVersion" } },
        { { u8"📄", u8"specVersion" } },
        { { u8"🪪", u8"meta" } },
        { { u8"👤", u8"humanoid" } },
        { { u8"👀", u8"firstPerson" } },
        { { u8"😀", u8"blendShapeMaster" } },
        { { u8"🔗", u8"secondaryAnimation" } },
        { { u8"💎", u8"materialProperties" } },
      } },
    },
    {
      // https : //
      // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.meta.schema.json
      u8"/extensions/VRM/meta",
      { {
        { { u8"🪪", u8"title" } },
        { { u8"🪪", u8"version" } },
        { { u8"🪪", u8"author" } },
        { { u8"🪪", u8"contactInformation" } },
        { { u8"🪪", u8"reference" } },
        { { u8"🆔", u8"texture" } },
        { { u8"🪪", u8"allowedUserName" } },
        { { u8"🪪", u8"violentUssageName" } },
        { { u8"🪪", u8"sexualUssageName" } },
        { { u8"🪪", u8"commercialUssageName" } },
        { { u8"🪪", u8"otherPermissionUrl" } },
        { { u8"🪪", u8"licenseName" } },
        { { u8"🪪", u8"otherLicenseUrl" } },
      } },
    },
    {
      // https : //
      // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.humanoid.schema.json
      u8"/extensions/VRM/humanoid",
      { {
        { { u8"🦴", u8"humanBones" } },
        { { u8"⛔", u8"armStretch" } },
        { { u8"⛔", u8"legStretch" } },
        { { u8"⛔", u8"upperArmTwist" } },
        { { u8"⛔", u8"lowerArmTwist" } },
        { { u8"⛔", u8"upperLegTwist" } },
        { { u8"⛔", u8"lowerLegTwist" } },
        { { u8"⛔", u8"feetSpacing" } },
        { { u8"⛔", u8"hasTranslationDoF" } },
      } },
    },
    {
      // https : //
      // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.humanoid.bone.schema.json
      u8"/extensions/VRM/humanoid/humanBones/*",
      { {
        { { u8"🦴", u8"bone" } },
        { { u8"🆔", u8"node" } },
        { { u8"⛔", u8"useDefaultValues" } },
        { { u8"⛔", u8"min" } },
        { { u8"⛔", u8"max" } },
        { { u8"⛔", u8"center" } },
        { { u8"⛔", u8"axisLength" } },
      } },
    },
    {
      // https : //
      // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.firstperson.schema.json
      u8"/extensions/VRM/firstPerson",
      { {
        { { u8"🆔", u8"firstPersonBone" } },
        { { u8"↔", u8"firstPersonBoneOffset" } },
        { { u8"✨", u8"meshAnnotations" } },
        { { u8"👀", u8"lookAtTypeName" } },
        { { u8"👀", u8"lookAtHorizontalInner" } },
        { { u8"👀", u8"lookAtHorizontalOuter" } },
        { { u8"👀", u8"lookAtVerticalDown" } },
        { { u8"👀", u8"lookAtVerticalUp" } },
      } },
    },
    {
      // https : //
      // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.blendshape.schema.json
      u8"/extensions/VRM/blendShapeMaster",
      { {
        { { u8"😀", u8"blendShapeGroups" } },
      } },
    },
    {
      // https : //
      // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.blendshape.group.schema.json
      u8"/extensions/VRM/blendShapeMaster/blendShapeGroups/*",
      { {
        { { u8"📄", u8"name" } },
        { { u8"😀", u8"presetName" } },
        { { u8"😀", u8"binds" } },
        { { u8"💎", u8"materialValues" } },
        { { u8"✅", u8"isBinary" } },
      } },
    },
    {
      // https : //
      // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.secondaryanimation.schema.json
      u8"/extensions/VRM/secondaryanimation",
      { {
        { { u8"🔗", u8"boneGroups" } },
        { { u8"🎱", u8"colliderGroups" } },
      } },
    },
    {
      // https : //
      // github.com/vrm-c/vrm-specification/blob/master/specification/0.0/schema/vrm.material.schema.json
      u8"/extensions/VRM/materialProperties/*",
      { {
        { { u8"📄", u8"name" } },
        { { u8"📄", u8"shader" } },
        { { u8"🔢", u8"renderQueue" } },
        { { u8"🔢", u8"floatProperties" } },
        { { u8"🔢", u8"vectorProperties" } },
        { { u8"🖼", u8"textureProperties" } },
        { { u8"📄", u8"keywordMap" } },
        { { u8"✅", u8"tagMap" } },
      } },
    },
  };
}

} // namespace
