#pragma once
#include "im_widgets.h"
#include "json_gui.h"
#include <gltfjson.h>
#include <gltfjson/jsonpath.h>

inline ShowGuiFunc
SelectSampler(std::u8string_view jsonpath)
{
  return [](const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            const gltfjson::tree::NodePtr& node) {
    if (SelectId("Sampler", node, root.Samplers.m_json)) {
      return true;
    } else {
      return false;
    }
  };
}

inline ShowGuiFunc
SelectTexture(std::u8string_view jsonpath)
{
  return [](const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            const gltfjson::tree::NodePtr& node) {
    if (SelectId("Source", node, root.Textures.m_json)) {
      return true;
    } else {
      return false;
    }
  };
}

struct FloatSlider
{
  float Min = 0;
  float Max = 1;
  float Default = 0.5f;

  ShowGuiFunc operator()(std::u8string_view jsonpath)
  {
    auto view = gltfjson::JsonPath(jsonpath).Back();
    std::u8string label{ view.begin(), view.end() };
    return [label, min = Min, max = Max, def = Default](
             const gltfjson::Root& root,
             const gltfjson::Bin& bin,
             const gltfjson::tree::NodePtr& node) {
      if (ShowGuiSliderFloat((const char*)label.c_str(), node, min, max, def)) {
        return true;
      } else {
        return false;
      }
    };
  }
};
