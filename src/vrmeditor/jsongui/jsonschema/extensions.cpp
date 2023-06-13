#pragma once
#include "extensions.h"

namespace jsonschema {

std::list<gltfjson::JsonPathMap<JsonSchema>::KeyValue> Extensions = {
  {
    u8"/extensions",
    { {
      { u8"VRM", u8"🌟", { {}, u8"{}" } },
      { u8"VRMC_vrm", u8"🌟", { {}, u8"{}" } },
      { u8"VRMC_springBone", u8"🌟", { {}, u8"{}" } },
    } },
  },
};

}
