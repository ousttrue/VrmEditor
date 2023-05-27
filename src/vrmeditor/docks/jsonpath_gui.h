#pragma once
#include "im_widgets.h"
#include "showgui.h"
#include <gltfjson.h>

inline ShowGuiFunc
SelectSampler(std::u8string_view jsonpath)
{
  return [](const gltfjson::typing::Root& root,
            const gltfjson::typing::Bin& bin,
            const gltfjson::tree::NodePtr& node) {
    if (SelectId("Sampler", node, root.Samplers.m_json)) {
      return true;
    } else {
      return false;
    }
  };
}
