#pragma once
#include <functional>
#include <gltfjson.h>

using ShowGuiFunc = std::function<bool(const gltfjson::Root& root,
                                       const gltfjson::Bin& bin,
                                       const gltfjson::tree::NodePtr&)>;

using CreateGuiFunc = std::function<ShowGuiFunc(std::u8string_view jsonpath)>;
