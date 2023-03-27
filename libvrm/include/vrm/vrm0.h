#pragma once
#include "humanoid.h"
#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

enum class ExpressionPreset {
  unknown,
  neutral,
  // lipsync
  a,
  i,
  u,
  e,
  o,
  // emotion
  joy,
  angry,
  sorrow,
  fun,
  // blink
  blink,
  blink_l,
  blink_r,
  // lookat
  lookup,
  lookdown,
  lookleft,
  lookright,
};

struct Vrm0ExpressionMorphTargetBind {
  // mesh index
  int mesh;
  // blendshape index
  int index;
  // max weight value(100)
  float weight;
};

struct ExpressionMaterialBind {};

struct Expression {
  ExpressionPreset preset;
  std::string name;
  bool isBinary;
  std::string label;
  float weight = 0;
  std::vector<Vrm0ExpressionMorphTargetBind> morphBinds;
  std::vector<ExpressionMaterialBind> materialBinds;

  Expression(std::string presetName, std::string_view name, bool isBinary)
      : name(name), isBinary(isBinary) {

    std::transform(presetName.begin(), presetName.end(), presetName.begin(),
                   ::tolower);

    if (presetName == "neutral") {
      preset = ExpressionPreset::neutral;
      label = presetName;
    } else if (presetName == "a") {
      preset = ExpressionPreset::a;
      label = presetName;
    } else if (presetName == "i") {
      preset = ExpressionPreset::i;
      label = presetName;
    } else if (presetName == "u") {
      preset = ExpressionPreset::u;
      label = presetName;
    } else if (presetName == "e") {
      preset = ExpressionPreset::e;
      label = presetName;
    } else if (presetName == "o") {
      preset = ExpressionPreset::o;
      label = presetName;
    } else if (presetName == "joy") {
      preset = ExpressionPreset::joy;
      label = presetName;
    } else if (presetName == "angry") {
      preset = ExpressionPreset::angry;
      label = presetName;
    } else if (presetName == "sorrow") {
      preset = ExpressionPreset::sorrow;
      label = presetName;
    } else if (presetName == "fun") {
      preset = ExpressionPreset::fun;
      label = presetName;
    } else if (presetName == "blink") {
      preset = ExpressionPreset::blink;
      label = presetName;
    } else if (presetName == "blink_l") {
      preset = ExpressionPreset::blink_l;
      label = presetName;
    } else if (presetName == "blink_r") {
      preset = ExpressionPreset::blink_r;
      label = presetName;
    } else if (presetName == "lookup") {
      preset = ExpressionPreset::lookup;
      label = presetName;
    } else if (presetName == "lookdown") {
      preset = ExpressionPreset::lookdown;
      label = presetName;
    } else if (presetName == "lookleft") {
      preset = ExpressionPreset::lookleft;
      label = presetName;
    } else if (presetName == "lookright") {
      preset = ExpressionPreset::lookright;
      label = presetName;
    } else {
      preset = ExpressionPreset::unknown;
      // TODO: must unique !
      label = name;
    }
  }
};

struct Vrm0 {

  vrm::Humanoid m_humanoid;

  std::vector<std::shared_ptr<Expression>> m_expressions;

  std::shared_ptr<Expression> addBlendShape(const std::string &presetName,
                                            std::string_view name,
                                            bool is_binary) {
    auto ptr = std::make_shared<Expression>(presetName, name, is_binary);
    m_expressions.push_back(ptr);
    return ptr;
  }
};
