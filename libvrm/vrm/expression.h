#pragma once
#include "node.h"
#include <algorithm>
#include <coroutine>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

namespace libvrm {

enum class ExpressionPreset
{
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
inline std::optional<ExpressionPreset>
ExpressionPresetFromVrm1String(std::u8string_view src)
{
  if (src == u8"neutral") {
    return ExpressionPreset::neutral;
  }
  if (src == u8"happy") { // vrm0: joy,
    return ExpressionPreset::happy;
  }
  if (src == u8"angry") { // vrm0: angry
    return ExpressionPreset::angry;
  }
  if (src == u8"sad") { // vrm0: sorrow,
    return ExpressionPreset::sad;
  }
  if (src == u8"relaxed") { // vrm0: fun,
    return ExpressionPreset::relaxed;
  }
  if (src == u8"surprised") { // vrm0: --
    return ExpressionPreset::surprised;
  }
  // lipsync(procesual)
  if (src == u8"aa") { // vrm0: a,
    return ExpressionPreset::aa;
  }
  if (src == u8"ih") { // vrm0: i,
    return ExpressionPreset::ih;
  }
  if (src == u8"ou") { // vrm0: u,
    return ExpressionPreset::ou;
  }
  if (src == u8"ee") { // vrm0: e,
    return ExpressionPreset::ee;
  }
  if (src == u8"oh") { // vrm0: o,
    return ExpressionPreset::oh;
  }
  // blink(procesual)
  if (src == u8"blink") { // vrm0: blink
    return ExpressionPreset::blink;
  }
  if (src == u8"blinkLeft") { // vrm0: blink_l,
    return ExpressionPreset::blinkLeft;
  }
  if (src == u8"blinkRight") { // vrm0: blink_r,
    return ExpressionPreset::blinkRight;
  }
  // lookat(procesual)
  if (src == u8"lookUp") { // vrm0: lookup,
    return ExpressionPreset::lookUp;
  }
  if (src == u8"lookDown") { // vrm0: lookdown,
    return ExpressionPreset::lookDown;
  }
  if (src == u8"lookLeft") { // vrm0: lookleft,
    return ExpressionPreset::lookLeft;
  }
  if (src == u8"lookRight") { // vrm0: lookright,
    return ExpressionPreset::lookRight;
  }

  return std::nullopt;
}
inline std::optional<ExpressionPreset>
ExpressionPresetFromVrm0String(std::u8string_view src)
{
  if (src == u8"neutral") {
    return ExpressionPreset::neutral;
  }
  if (src == u8"joy") { // vrm0: joy,
    return ExpressionPreset::happy;
  }
  if (src == u8"angry") { // vrm0: angry
    return ExpressionPreset::angry;
  }
  if (src == u8"sorrow") { // vrm0: sorrow,
    return ExpressionPreset::sad;
  }
  if (src == u8"fun") { // vrm0: fun,
    return ExpressionPreset::relaxed;
  }
  if (src == u8"surprised") { // vrm0: --
    return ExpressionPreset::surprised;
  }
  // lipsync(procesual)
  if (src == u8"a") { // vrm0: a,
    return ExpressionPreset::aa;
  }
  if (src == u8"i") { // vrm0: i,
    return ExpressionPreset::ih;
  }
  if (src == u8"u") { // vrm0: u,
    return ExpressionPreset::ou;
  }
  if (src == u8"e") { // vrm0: e,
    return ExpressionPreset::ee;
  }
  if (src == u8"o") { // vrm0: o,
    return ExpressionPreset::oh;
  }
  // blink(procesual)
  if (src == u8"blink") { // vrm0: blink
    return ExpressionPreset::blink;
  }
  if (src == u8"blink_l") { // vrm0: blink_l,
    return ExpressionPreset::blinkLeft;
  }
  if (src == u8"blink_r") { // vrm0: blink_r,
    return ExpressionPreset::blinkRight;
  }
  // lookat(procesual)
  if (src == u8"lookup") { // vrm0: lookup,
    return ExpressionPreset::lookUp;
  }
  if (src == u8"lookdown") { // vrm0: lookdown,
    return ExpressionPreset::lookDown;
  }
  if (src == u8"lookleft") { // vrm0: lookleft,
    return ExpressionPreset::lookLeft;
  }
  if (src == u8"lookright") { // vrm0: lookright,
    return ExpressionPreset::lookRight;
  }

  return std::nullopt;
}
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
struct std::hash<libvrm::MorphTargetKey>
{
  std::size_t operator()(const libvrm::MorphTargetKey& key) const
  {
    return key.Hash;
  }
};

namespace libvrm {
struct ExpressionMorphTargetBind
{
  // mesh index
  int mesh;
  std::shared_ptr<Node> Node;
  // blendshape index
  int index;
  // max weight value(100)
  float weight;
};

struct ExpressionMaterialBind
{};

using NodeToIndexFunc =
  std::function<uint32_t(const std::shared_ptr<Node>& node)>;

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
  std::unordered_map<MorphTargetKey, float> m_morphTargetMap;

  // preset
  Expression Happy;
  Expression Angry;
  Expression Sad;
  Expression Relaxed;
  Expression Surprised;
  // blink(procesual)
  Expression Blink;
  Expression BlinkLeft;
  Expression BlinkRight;
  // lipsync(procesual)
  Expression Aa;
  Expression Ih;
  Expression Ou;
  Expression Ee;
  Expression Oh;
  // lookat(procesual)
  Expression LookUp;
  Expression LookDown;
  Expression LookLeft;
  Expression LookRight;
  // other
  Expression Neutral;

  Expression Error = {};

  std::list<Expression> CustomExpressions;

  struct generator
  {
    struct promise_type
    {
      Expression* m_value;
      auto get_return_object()
      {
        return generator{ std::coroutine_handle<promise_type>::from_promise(
          *this) };
      };
      auto initial_suspend() { return std::suspend_always{}; }
      auto final_suspend() noexcept { return std::suspend_always{}; }
      auto yield_value(Expression* v)
      {
        m_value = v;
        return std::suspend_always{};
      }
      void return_void() {}
      void unhandled_exception() { std::terminate(); }
    };
    std::coroutine_handle<promise_type> m_handle;
    struct iterator
    {
      std::coroutine_handle<promise_type> m_handle;
      bool m_done;
      iterator& operator++()
      {
        m_handle.resume();
        m_done = m_handle.done();
        return *this;
      }
      bool operator!=(const iterator& rhs) const
      {
        return m_done != rhs.m_done;
      }

      Expression* operator*() const { return m_handle.promise().m_value; }
    };
    iterator begin()
    {
      m_handle.resume();
      return { m_handle, m_handle.done() };
    }
    iterator end() { return { {}, true }; }
  };

  generator Enumerate()
  {
    co_yield &Happy;
    co_yield &Angry;
    co_yield &Sad;
    co_yield &Relaxed;
    co_yield &Surprised;
    // blink(procesual)
    co_yield &Blink;
    co_yield &BlinkLeft;
    co_yield &BlinkRight;
    // lipsync(procesual)
    co_yield &Aa;
    co_yield &Ih;
    co_yield &Ou;
    co_yield &Ee;
    co_yield &Oh;
    // lookat(procesual)
    co_yield &LookUp;
    co_yield &LookDown;
    co_yield &LookLeft;
    co_yield &LookRight;
    // other
    co_yield &Neutral;

    for (auto& e : CustomExpressions) {
      co_yield &e;
    }
  }

  Expression& Preset(ExpressionPreset preset)
  {
    switch (preset) {
      case ExpressionPreset::neutral:
        return Neutral;
      case ExpressionPreset::happy:
        return Happy;
      case ExpressionPreset::angry:
        return Angry;
      case ExpressionPreset::sad:
        return Sad;
      case ExpressionPreset::relaxed:
        return Relaxed;
      case ExpressionPreset::surprised:
        return Surprised;
      case ExpressionPreset::aa:
        return Aa;
      case ExpressionPreset::ih:
        return Ih;
      case ExpressionPreset::ou:
        return Ou;
      case ExpressionPreset::ee:
        return Ee;
      case ExpressionPreset::oh:
        return Oh;
      case ExpressionPreset::blink:
        return Blink;
      case ExpressionPreset::blinkLeft:
        return BlinkLeft;
      case ExpressionPreset::blinkRight:
        return BlinkRight;
      case ExpressionPreset::lookUp:
        return LookUp;
      case ExpressionPreset::lookDown:
        return LookDown;
      case ExpressionPreset::lookLeft:
        return LookLeft;
      case ExpressionPreset::lookRight:
        return LookRight;
    }

    assert(false);
    return Error;
  }

  // Expression createExpression(const std::u8string& _presetName,
  //                             const std::u8string& name,
  //                             bool isBinary)
  // {
  //   Expression ex{
  //     .name = { (const char*)name.data(), name.size() },
  //     .isBinary = isBinary,
  //   };
  //
  //   // tolower
  //   std::string presetName{ (const char*)_presetName.data(),
  //                           _presetName.size() };
  //   std::transform(
  //     presetName.begin(), presetName.end(), presetName.begin(), ::tolower);
  //
  //   if (presetName == "neutral") {
  //     ex.preset = ExpressionPreset::neutral;
  //     ex.label = presetName;
  //   } else if (presetName == "a") {
  //     ex.preset = ExpressionPreset::aa;
  //     ex.label = presetName;
  //   } else if (presetName == "i") {
  //     ex.preset = ExpressionPreset::ih;
  //     ex.label = presetName;
  //   } else if (presetName == "u") {
  //     ex.preset = ExpressionPreset::ou;
  //     ex.label = presetName;
  //   } else if (presetName == "e") {
  //     ex.preset = ExpressionPreset::ee;
  //     ex.label = presetName;
  //   } else if (presetName == "o") {
  //     ex.preset = ExpressionPreset::oh;
  //     ex.label = presetName;
  //   } else if (presetName == "joy" || presetName == "happy") {
  //     ex.preset = ExpressionPreset::happy;
  //     ex.label = presetName;
  //   } else if (presetName == "angry") {
  //     ex.preset = ExpressionPreset::angry;
  //     ex.label = presetName;
  //   } else if (presetName == "sorrow" || presetName == "sad") {
  //     ex.preset = ExpressionPreset::sad;
  //     ex.label = presetName;
  //   } else if (presetName == "fun" || presetName == "relaxed") {
  //     ex.preset = ExpressionPreset::relaxed;
  //     ex.label = presetName;
  //   } else if (presetName == "surprised") {
  //     ex.preset = ExpressionPreset::surprised;
  //     ex.label = presetName;
  //   } else if (presetName == "blink") {
  //     ex.preset = ExpressionPreset::blink;
  //     ex.label = presetName;
  //   } else if (presetName == "blink_l") {
  //     ex.preset = ExpressionPreset::blinkLeft;
  //     ex.label = presetName;
  //   } else if (presetName == "blink_r") {
  //     ex.preset = ExpressionPreset::blinkRight;
  //     ex.label = presetName;
  //   } else if (presetName == "lookup") {
  //     ex.preset = ExpressionPreset::lookUp;
  //     ex.label = presetName;
  //   } else if (presetName == "lookdown") {
  //     ex.preset = ExpressionPreset::lookDown;
  //     ex.label = presetName;
  //   } else if (presetName == "lookleft") {
  //     ex.preset = ExpressionPreset::lookLeft;
  //     ex.label = presetName;
  //   } else if (presetName == "lookright") {
  //     ex.preset = ExpressionPreset::lookRight;
  //     ex.label = presetName;
  //   } else {
  //     ex.preset = ExpressionPreset::unknown;
  //     // TODO: must unique !
  //     ex.label = { (const char*)name.data(), name.size() };
  //   }
  //   return ex;
  // }

  // std::shared_ptr<Expression> addBlendShape(const std::u8string& presetName,
  //                                           const std::u8string& name,
  //                                           bool is_binary)
  // {
  //   auto ex = std::make_shared<Expression>();
  //   *ex = createExpression(presetName, name, is_binary);
  //   Expressions.push_back(ex);
  //   return Expressions.back();
  // }

  const std::unordered_map<MorphTargetKey, float>& EvalMorphTargetMap(
    const NodeToIndexFunc& nodeToIndex)
  {
    // clear
    m_morphTargetMap.clear();
    // apply
    for (auto expression : Enumerate()) {
      ApplyExpression(expression, nodeToIndex);
    }
    return m_morphTargetMap;
  }

  void ApplyExpression(const Expression* expression,
                       const NodeToIndexFunc& nodeToIndex)
  {
    for (auto& bind : expression->morphBinds) {
      auto nodeIndex = nodeToIndex(bind.Node);
      if (nodeIndex == -1) {
        continue;
      }
      MorphTargetKey key{
        .NodeIndex = static_cast<uint16_t>(nodeIndex),
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

} // namespace
