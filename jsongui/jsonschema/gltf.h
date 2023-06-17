#pragma once
#include "json_prop.h"
#include <gltfjson/jsonpath.h>

namespace jsonschema {

std::list<gltfjson::JsonPathMap<JsonSchema>::KeyValue> Gltf();

} // namespace
