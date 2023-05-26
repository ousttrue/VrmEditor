#pragma once
#include <functional>
#include <gltfjson.h>

using ShowGuiFunc = std::function<void(const gltfjson::typing::Root& root,
                                       const gltfjson::typing::Bin& bin,
                                       const gltfjson::tree::NodePtr&)>;

using CreateGuiFunc = std::function<ShowGuiFunc(std::u8string_view jsonpath)>;
