#pragma once
#include "extensions.h"
#include "../json_widgets.h"

namespace jsonschema {

std::list<gltfjson::JsonPathMap<JsonSchema>::KeyValue>
Extensions()
{

  return {
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

} // namespace
