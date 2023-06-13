#pragma once
#include "../json_widgets.h"
#include "json_prop.h"
#include <gltfjson/jsonpath.h>
#include <string>

namespace jsonschema {

extern std::list<gltfjson::JsonPathMap<JsonSchema>::KeyValue> Extensions;

} // namespace
