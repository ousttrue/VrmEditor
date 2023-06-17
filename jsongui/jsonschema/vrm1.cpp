#include "vrm1.h"
#include "../json_widgets.h"

namespace jsonschema {

std::list<gltfjson::JsonPathMap<JsonSchema>::KeyValue>
VRMC_vrm()
{
  JsonSchema HumanBone{ {
    { { u8"🆔", u8"node" }, JsonValue::Number },
  } };
  JsonSchema HumanBones{
    {
      { { u8"🦴", u8"hips" }, JsonValue::Object },
      { { u8"🦴", u8"spine" }, JsonValue::Object },
      { { u8"🦴", u8"chest" }, JsonValue::Object },
      { { u8"🦴", u8"upperChest" }, JsonValue::Object },
      { { u8"🦴", u8"neck" }, JsonValue::Object },
      { { u8"🦴", u8"head" }, JsonValue::Object },
      { { u8"🦴", u8"jaw" }, JsonValue::Object },
      { { u8"🦴", u8"leftUpperLeg" }, JsonValue::Object },
      { { u8"🦴", u8"leftLowerLeg" }, JsonValue::Object },
      { { u8"🦴", u8"leftFoot" }, JsonValue::Object },
      { { u8"🦴", u8"leftToes" }, JsonValue::Object },
      { { u8"🦴", u8"rightUpperLeg" }, JsonValue::Object },
      { { u8"🦴", u8"rightLowerLeg" }, JsonValue::Object },
      { { u8"🦴", u8"rightFoot" }, JsonValue::Object },
      { { u8"🦴", u8"rightToes" }, JsonValue::Object },
      { { u8"🦴", u8"leftShoulder" }, JsonValue::Object },
      { { u8"🦴", u8"leftUpperArm" }, JsonValue::Object },
      { { u8"🦴", u8"leftLowerArm" }, JsonValue::Object },
      { { u8"🦴", u8"leftHand" }, JsonValue::Object },
      { { u8"🦴", u8"rightShoulder" }, JsonValue::Object },
      { { u8"🦴", u8"rightUpperArm" }, JsonValue::Object },
      { { u8"🦴", u8"rightLowerArm" }, JsonValue::Object },
      { { u8"🦴", u8"rightHand" }, JsonValue::Object },
      { { u8"🦴", u8"leftThumbMetacarpal" }, JsonValue::Object },
      { { u8"🦴", u8"leftThumbProximal" }, JsonValue::Object },
      { { u8"🦴", u8"leftThumbDistal" }, JsonValue::Object },
      { { u8"🦴", u8"leftIndexProximal" }, JsonValue::Object },
      { { u8"🦴", u8"leftIndexIntermediate" }, JsonValue::Object },
      { { u8"🦴", u8"leftIndexDistal" }, JsonValue::Object },
      { { u8"🦴", u8"leftMiddleProximal" }, JsonValue::Object },
      { { u8"🦴", u8"leftMiddleIntermediate" }, JsonValue::Object },
      { { u8"🦴", u8"leftMiddleDistal" }, JsonValue::Object },
      { { u8"🦴", u8"leftRingProximal" }, JsonValue::Object },
      { { u8"🦴", u8"leftRingIntermediate" }, JsonValue::Object },
      { { u8"🦴", u8"leftRingDistal" }, JsonValue::Object },
      { { u8"🦴", u8"leftLittleProximal" }, JsonValue::Object },
      { { u8"🦴", u8"leftLittleIntermediate" }, JsonValue::Object },
      { { u8"🦴", u8"leftLittleDistal" }, JsonValue::Object },
      { { u8"🦴", u8"rightThumbMetacarpal" }, JsonValue::Object },
      { { u8"🦴", u8"rightThumbProximal" }, JsonValue::Object },
      { { u8"🦴", u8"rightThumbDistal" }, JsonValue::Object },
      { { u8"🦴", u8"rightIndexProximal" }, JsonValue::Object },
      { { u8"🦴", u8"rightIndexIntermediate" }, JsonValue::Object },
      { { u8"🦴", u8"rightIndexDistal" }, JsonValue::Object },
      { { u8"🦴", u8"rightMiddleProximal" }, JsonValue::Object },
      { { u8"🦴", u8"rightMiddleIntermediate" }, JsonValue::Object },
      { { u8"🦴", u8"rightMiddleDistal" }, JsonValue::Object },
      { { u8"🦴", u8"rightRingProximal" }, JsonValue::Object },
      { { u8"🦴", u8"rightRingIntermediate" }, JsonValue::Object },
      { { u8"🦴", u8"rightRingDistal" }, JsonValue::Object },
      { { u8"🦴", u8"rightLittleProximal" }, JsonValue::Object },
      { { u8"🦴", u8"rightLittleIntermediate" }, JsonValue::Object },
      { { u8"🦴", u8"rightLittleDistal" }, JsonValue::Object },
    },
  };
  JsonSchema Humanoid{
    {
      { { u8"🦴", u8"humanBones" }, JsonValue::Object },
    },
  };
  JsonSchema AnimationHumanBones{
    {
      { { u8"🦴", u8"hips" }, JsonValue::Object },
      { { u8"🦴", u8"spine" }, JsonValue::Object },
      { { u8"🦴", u8"chest" }, JsonValue::Object },
      { { u8"🦴", u8"upperChest" }, JsonValue::Object },
      { { u8"🦴", u8"neck" }, JsonValue::Object },
      { { u8"🦴", u8"head" }, JsonValue::Object },
      { { u8"🦴", u8"jaw" }, JsonValue::Object },
      { { u8"🦴", u8"leftUpperLeg" }, JsonValue::Object },
      { { u8"🦴", u8"leftLowerLeg" }, JsonValue::Object },
      { { u8"🦴", u8"leftFoot" }, JsonValue::Object },
      { { u8"🦴", u8"leftToes" }, JsonValue::Object },
      { { u8"🦴", u8"rightUpperLeg" }, JsonValue::Object },
      { { u8"🦴", u8"rightLowerLeg" }, JsonValue::Object },
      { { u8"🦴", u8"rightFoot" }, JsonValue::Object },
      { { u8"🦴", u8"rightToes" }, JsonValue::Object },
      { { u8"🦴", u8"leftShoulder" }, JsonValue::Object },
      { { u8"🦴", u8"leftUpperArm" }, JsonValue::Object },
      { { u8"🦴", u8"leftLowerArm" }, JsonValue::Object },
      { { u8"🦴", u8"leftHand" }, JsonValue::Object },
      { { u8"🦴", u8"rightShoulder" }, JsonValue::Object },
      { { u8"🦴", u8"rightUpperArm" }, JsonValue::Object },
      { { u8"🦴", u8"rightLowerArm" }, JsonValue::Object },
      { { u8"🦴", u8"rightHand" }, JsonValue::Object },
      { { u8"🦴", u8"leftThumbMetacarpal" }, JsonValue::Object },
      { { u8"🦴", u8"leftThumbProximal" }, JsonValue::Object },
      { { u8"🦴", u8"leftThumbDistal" }, JsonValue::Object },
      { { u8"🦴", u8"leftIndexProximal" }, JsonValue::Object },
      { { u8"🦴", u8"leftIndexIntermediate" }, JsonValue::Object },
      { { u8"🦴", u8"leftIndexDistal" }, JsonValue::Object },
      { { u8"🦴", u8"leftMiddleProximal" }, JsonValue::Object },
      { { u8"🦴", u8"leftMiddleIntermediate" }, JsonValue::Object },
      { { u8"🦴", u8"leftMiddleDistal" }, JsonValue::Object },
      { { u8"🦴", u8"leftRingProximal" }, JsonValue::Object },
      { { u8"🦴", u8"leftRingIntermediate" }, JsonValue::Object },
      { { u8"🦴", u8"leftRingDistal" }, JsonValue::Object },
      { { u8"🦴", u8"leftLittleProximal" }, JsonValue::Object },
      { { u8"🦴", u8"leftLittleIntermediate" }, JsonValue::Object },
      { { u8"🦴", u8"leftLittleDistal" }, JsonValue::Object },
      { { u8"🦴", u8"rightThumbMetacarpal" }, JsonValue::Object },
      { { u8"🦴", u8"rightThumbProximal" }, JsonValue::Object },
      { { u8"🦴", u8"rightThumbDistal" }, JsonValue::Object },
      { { u8"🦴", u8"rightIndexProximal" }, JsonValue::Object },
      { { u8"🦴", u8"rightIndexIntermediate" }, JsonValue::Object },
      { { u8"🦴", u8"rightIndexDistal" }, JsonValue::Object },
      { { u8"🦴", u8"rightMiddleProximal" }, JsonValue::Object },
      { { u8"🦴", u8"rightMiddleIntermediate" }, JsonValue::Object },
      { { u8"🦴", u8"rightMiddleDistal" }, JsonValue::Object },
      { { u8"🦴", u8"rightRingProximal" }, JsonValue::Object },
      { { u8"🦴", u8"rightRingIntermediate" }, JsonValue::Object },
      { { u8"🦴", u8"rightRingDistal" }, JsonValue::Object },
      { { u8"🦴", u8"rightLittleProximal" }, JsonValue::Object },
      { { u8"🦴", u8"rightLittleIntermediate" }, JsonValue::Object },
      { { u8"🦴", u8"rightLittleDistal" }, JsonValue::Object },
    },
  };
  static std::list<gltfjson::JsonPathMap<JsonSchema>::KeyValue> s_map{
    {
      // https : //
      // github.com/vrm-c/vrm-specification/blob/master/specification/VRMC_vrm-1.0/schema/VRMC_vrm.schema.json
      u8"/extensions/VRMC_vrm",
      { {
        { { u8"📄", u8"specVersion" }, {}, JsonPropFlags::Required },
        { { u8"🪪", u8"meta" } },
        { { u8"👤", u8"humanoid" } },
        { { u8"✨", u8"firstPerson" } },
        { { u8"👀", u8"lookAt" } },
        { { u8"😀", u8"expressions" } },
      } },
    },
    {
      u8"/extensions/VRMC_vrm/humanoid",
      Humanoid,
    },
    {
      u8"/extensions/VRMC_vrm/humanoid/humanBones",
      HumanBones,
    },
    {
      u8"/extensions/VRMC_vrm/humanoid/humanBones/*",
      HumanBone,
    },
    {
      // https://github.com/vrm-c/vrm-specification/blob/master/specification/VRMC_vrm-1.0/schema/VRMC_vrm.expressions.schema.json
      u8"/extensions/VRMC_vrm/expressions/preset",
      { {
        { { u8"😆", u8"happy" } },
        { { u8"😠", u8"angry" } },
        { { u8"😥", u8"sad" } },
        { { u8"🙂", u8"relaxed" } },
        { { u8"😯", u8"surprised" } },
        { { u8"👄", u8"aa" } },
        { { u8"👄", u8"ih" } },
        { { u8"👄", u8"ou" } },
        { { u8"👄", u8"ee" } },
        { { u8"👄", u8"oh" } },
        { { u8"😉", u8"blink" } },
        { { u8"😉", u8"blinkLeft" } },
        { { u8"😉", u8"blinkRight" } },
        { { u8"👀", u8"lookUp" } },
        { { u8"👀", u8"lookDown" } },
        { { u8"👀", u8"lookLeft" } },
        { { u8"👀", u8"lookRight" } },
        { { u8"😶", u8"neutral" } },
      } },
    },
    {
      // https://github.com/vrm-c/vrm-specification/blob/master/specification/VRMC_vrm_animation-1.0/schema/VRMC_vrm_animation.schema.json
      u8"/extensions/VRMC_vrm_animation",
      { {
        { { u8"📄", u8"specVersion" }, {}, JsonPropFlags::Required },
        { { u8"👤", u8"humanoid" }, JsonValue::Object },
      } },
    },
    {
      u8"/extensions/VRMC_vrm_animation/humanoid",
      Humanoid,
    },
    {
      u8"/extensions/VRMC_vrm_animation/humanoid/humanBones",
      AnimationHumanBones,
    },
    {
      u8"/extensions/VRMC_vrm_animation/humanoid/humanBones/*",
      HumanBone,
    },
  };
  return s_map;
}

}
