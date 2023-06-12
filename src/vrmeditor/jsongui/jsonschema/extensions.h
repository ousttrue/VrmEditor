#pragma once
#include "../json_widgets.h"
#include "json_prop.h"
#include <gltfjson/jsonpath.h>
#include <string>

namespace jsonschema {

inline std::list<gltfjson::JsonPathMap<JsonSchema>::KeyValue> Extensions = {
  {
    u8"/extensions",
    { {
      { u8"VRM", u8"🌟", { {}, u8"{}" } },
      { u8"VRMC_vrm", u8"🌟", { {}, u8"{}" } },
      { u8"VRMC_springBone", u8"🌟", { {}, u8"{}" } },
    } },
  },
};

} // namespace
