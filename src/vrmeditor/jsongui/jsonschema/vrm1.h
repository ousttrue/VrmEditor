#pragma once
#include "../json_widgets.h"
#include "json_prop.h"
#include <gltfjson/jsonpath.h>
#include <string>

namespace jsonschema {

const std::list<gltfjson::JsonPathMap<JsonSchema>::KeyValue>&
VRMC_vrm();

}
