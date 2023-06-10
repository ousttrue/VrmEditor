#pragma once
#include "json_gui.h"
#include "json_widgets.h"
#include <array>
#include <gltfjson.h>
#include <gltfjson/jsonpath.h>
#include <span>
#include <vector>

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

struct RgbPicker
{
  std::array<float, 3> Default = { 1, 1, 1 };

  ShowGuiFunc operator()(std::u8string_view jsonpath)
  {
    auto view = gltfjson::JsonPath(jsonpath).Back();
    std::u8string label{ view.begin(), view.end() };
    return [label, def = Default](const gltfjson::Root& root,
                                  const gltfjson::Bin& bin,
                                  const gltfjson::tree::NodePtr& node) {
      if (ShowGuiColor3((const char*)label.c_str(), node, def)) {
        return true;
      } else {
        return false;
      }
    };
  }
};

struct RgbaPicker
{
  std::array<float, 4> Default = { 1, 1, 1, 1 };

  ShowGuiFunc operator()(std::u8string_view jsonpath)
  {
    auto view = gltfjson::JsonPath(jsonpath).Back();
    std::u8string label{ view.begin(), view.end() };
    return [label, def = Default](const gltfjson::Root& root,
                                  const gltfjson::Bin& bin,
                                  const gltfjson::tree::NodePtr& node) {
      if (ShowGuiColor4((const char*)label.c_str(), node, def)) {
        return true;
      } else {
        return false;
      }
    };
  }
};

struct StringEnum
{
  std::vector<const char*> Values;

  ShowGuiFunc operator()(std::u8string_view jsonpath)
  {
    auto view = gltfjson::JsonPath(jsonpath).Back();
    std::u8string label{ view.begin(), view.end() };
    // std::span<const char*> values = Values;
    return [label, &values = Values](const gltfjson::Root& root,
                                     const gltfjson::Bin& bin,
                                     const gltfjson::tree::NodePtr& node) {
      if (ShowGuiStringEnum((const char*)label.c_str(), node, values)) {
        return true;
      } else {
        return false;
      }
    };
  }
};
