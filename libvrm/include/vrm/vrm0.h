#pragma once
#include "humanoid.h"
#include "springbone.h"
#include <algorithm>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace vrm0 {
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

struct ExpressionMorphTargetBind {
  // mesh index
  int mesh;
  // blendshape index
  int index;
  // max weight value(100)
  float weight;
};
inline void from_json(const nlohmann::json &j, ExpressionMorphTargetBind &b) {
  b.mesh = j.at("mesh");
  b.index = j.at("index");
  b.weight = j.at("weight");
}

struct ExpressionMaterialBind {};

struct Expression {
  ExpressionPreset preset;
  std::string name;
  bool isBinary;
  std::string label;
  float weight = 0;
  std::vector<ExpressionMorphTargetBind> morphBinds;
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

// using MorphTargetKey = std::tuple<uint16_t, uint16_t>;
union MorphTargetKey {
  struct {
    uint16_t NodeIndex;
    uint16_t MorphIndex;
  };
  uint32_t Hash;

  bool operator==(const MorphTargetKey &rhs) const { return Hash == rhs.Hash; }
};
} // namespace vrm0

template <> struct std::hash<vrm0::MorphTargetKey> {
  std::size_t operator()(const vrm0::MorphTargetKey &key) const {
    return key.Hash;
  }
};

namespace vrm0 {
struct Vrm {

  vrm::Humanoid m_humanoid;
  std::vector<std::shared_ptr<Expression>> m_expressions;
  std::vector<std::shared_ptr<ColliderGroup>> m_colliderGroups;
  std::vector<std::shared_ptr<Spring>> m_springs;
  std::unordered_map<MorphTargetKey, float> m_morphTargetMap;

  std::shared_ptr<Expression> addBlendShape(const std::string &presetName,
                                            std::string_view name,
                                            bool is_binary) {
    auto ptr = std::make_shared<Expression>(presetName, name, is_binary);
    m_expressions.push_back(ptr);
    return ptr;
  }

  const std::unordered_map<MorphTargetKey, float> &
  EvalMorphTargetMap(const std::function<uint32_t(uint32_t)> &MeshToNode) {
    // clear
    m_morphTargetMap.clear();
    // apply
    for (auto &expression : m_expressions) {
      for (auto &bind : expression->morphBinds) {
        MorphTargetKey key{
            .NodeIndex = static_cast<uint16_t>(MeshToNode(bind.mesh)),
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
    return m_morphTargetMap;
  }
};

} // namespace vrm0
