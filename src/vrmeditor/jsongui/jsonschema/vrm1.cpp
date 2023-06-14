#include "vrm1.h"
#include "../json_widgets.h"

namespace jsonschema {

std::list<gltfjson::JsonPathMap<JsonSchema>::KeyValue>
VRMC_vrm()
{
  static std::list<gltfjson::JsonPathMap<JsonSchema>::KeyValue> s_map{
    {
      // https : //
      // github.com/vrm-c/vrm-specification/blob/master/specification/VRMC_vrm-1.0/schema/VRMC_vrm.schema.json
      u8"/extensions/VRMC_vrm",
      { {
        { { u8"📄", u8"specVersion" } },
        { { u8"🪪", u8"meta" } },
        { { u8"👤", u8"humanoid" } },
        { { u8"✨", u8"firstPerson" } },
        { { u8"👀", u8"lookAt" } },
        { { u8"😀", u8"expressions" } },
      } },
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
  };
  return s_map;
}

}
