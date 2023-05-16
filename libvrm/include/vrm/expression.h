#pragma once
#include "node.h"
#include <functional>

namespace libvrm::vrm {

enum class ExpressionPreset
{
  unknown,
  neutral,
  // emotion
  happy,     // vrm0: joy,
  angry,     // vrm0: angry
  sad,       // vrm0: sorrow,
  relaxed,   // vrm0: fun,
  surprised, // vrm0: --
  // lipsync(procesual)
  aa, // vrm0: a,
  ih, // vrm0: i,
  ou, // vrm0: u,
  ee, // vrm0: e,
  oh, // vrm0: o,
  // blink(procesual)
  blink,      // vrm0: blink
  blinkLeft,  // vrm0: blink_l,
  blinkRight, // vrm0: blink_r,
  // lookat(procesual)
  lookUp,    // vrm0: lookup,
  lookDown,  // vrm0: lookdown,
  lookLeft,  // vrm0: lookleft,
  lookRight, // vrm0: lookright,
};

// using MorphTargetKey = std::tuple<uint16_t, uint16_t>;
union MorphTargetKey
{
  struct
  {
    uint16_t NodeIndex;
    uint16_t MorphIndex;
  };
  uint32_t Hash;

  bool operator==(const MorphTargetKey& rhs) const { return Hash == rhs.Hash; }
};
}

template<>
struct std::hash<libvrm::vrm::MorphTargetKey>
{
  std::size_t operator()(const libvrm::vrm::MorphTargetKey& key) const
  {
    return key.Hash;
  }
};

namespace libvrm::vrm {
struct ExpressionMorphTargetBind
{
  // mesh index
  int mesh;
  std::shared_ptr<gltf::Node> Node;
  // blendshape index
  int index;
  // max weight value(100)
  float weight;
};
// inline void
// from_json(const nlohmann::json& j, ExpressionMorphTargetBind& b)
// {
//   b.mesh = j.at("mesh");
//   b.index = j.at("index");
//   b.weight = j.at("weight");
// }

struct ExpressionMaterialBind
{};

using NodeToIndexFunc =
  std::function<uint32_t(const std::shared_ptr<gltf::Node>& node)>;

struct Expression
{
  ExpressionPreset preset;
  std::string name;
  bool isBinary;
  std::string label;
  float weight = 0;
  std::vector<ExpressionMorphTargetBind> morphBinds;
  std::vector<ExpressionMaterialBind> materialBinds;

  bool Empty() const
  {
    if (morphBinds.size())
      return false;
    if (materialBinds.size())
      return false;
    return true;
  }
};

struct Expressions
{
  std::shared_ptr<Expression> Get(ExpressionPreset preset) const
  {
    for (auto& ex : Expressions)
      if (ex->preset == preset)
        return ex;
    return nullptr;
  }

  std::unordered_map<MorphTargetKey, float> m_morphTargetMap;

  std::vector<std::shared_ptr<Expression>> Expressions;

  Expression createExpression(std::string presetName,
                              std::string_view name,
                              bool isBinary)
  {
    Expression ex{
      .name = { name.begin(), name.end() },
      .isBinary = isBinary,
    };

    // tolower
    std::transform(
      presetName.begin(), presetName.end(), presetName.begin(), ::tolower);

    if (presetName == "neutral") {
      ex.preset = ExpressionPreset::neutral;
      ex.label = presetName;
    } else if (presetName == "a") {
      ex.preset = ExpressionPreset::aa;
      ex.label = presetName;
    } else if (presetName == "i") {
      ex.preset = ExpressionPreset::ih;
      ex.label = presetName;
    } else if (presetName == "u") {
      ex.preset = ExpressionPreset::ou;
      ex.label = presetName;
    } else if (presetName == "e") {
      ex.preset = ExpressionPreset::ee;
      ex.label = presetName;
    } else if (presetName == "o") {
      ex.preset = ExpressionPreset::oh;
      ex.label = presetName;
    } else if (presetName == "joy") {
      ex.preset = ExpressionPreset::happy;
      ex.label = presetName;
    } else if (presetName == "angry") {
      ex.preset = ExpressionPreset::angry;
      ex.label = presetName;
    } else if (presetName == "sorrow") {
      ex.preset = ExpressionPreset::sad;
      ex.label = presetName;
    } else if (presetName == "fun") {
      ex.preset = ExpressionPreset::relaxed;
      ex.label = presetName;
    } else if (presetName == "blink") {
      ex.preset = ExpressionPreset::blink;
      ex.label = presetName;
    } else if (presetName == "blink_l") {
      ex.preset = ExpressionPreset::blinkLeft;
      ex.label = presetName;
    } else if (presetName == "blink_r") {
      ex.preset = ExpressionPreset::blinkRight;
      ex.label = presetName;
    } else if (presetName == "lookup") {
      ex.preset = ExpressionPreset::lookUp;
      ex.label = presetName;
    } else if (presetName == "lookdown") {
      ex.preset = ExpressionPreset::lookDown;
      ex.label = presetName;
    } else if (presetName == "lookleft") {
      ex.preset = ExpressionPreset::lookLeft;
      ex.label = presetName;
    } else if (presetName == "lookright") {
      ex.preset = ExpressionPreset::lookRight;
      ex.label = presetName;
    } else {
      ex.preset = ExpressionPreset::unknown;
      // TODO: must unique !
      ex.label = name;
    }
    return ex;
  }
  std::shared_ptr<Expression> addBlendShape(const std::string& presetName,
                                            std::string_view name,
                                            bool is_binary)
  {
    auto ex = std::make_shared<Expression>();
    *ex = createExpression(presetName, name, is_binary);
    Expressions.push_back(ex);
    return Expressions.back();
  }

  const std::unordered_map<MorphTargetKey, float>& EvalMorphTargetMap(
    const NodeToIndexFunc& nodeToIndex)
  {
    // clear
    m_morphTargetMap.clear();
    // apply
    for (auto& expression : Expressions) {
      ApplyExpression(expression, nodeToIndex);
    }
    return m_morphTargetMap;
  }

  void ApplyExpression(const std::shared_ptr<Expression>& expression,
                       const NodeToIndexFunc& nodeToIndex)
  {
    for (auto& bind : expression->morphBinds) {
      MorphTargetKey key{
        .NodeIndex = static_cast<uint16_t>(nodeToIndex(bind.Node)),
        .MorphIndex = static_cast<uint16_t>(bind.index),
      };
      auto found = m_morphTargetMap.find(key);
      auto weight = bind.weight * expression->weight;
      if (found != m_morphTargetMap.end()) {
        found->second += weight;
      } else {
        m_morphTargetMap.insert(std::make_pair(key, weight));
      }
    }
  }
};

}
