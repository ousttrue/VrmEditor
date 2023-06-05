#pragma once
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <vrm/animation/timeline.h>
#include <vrm/humanoid/humanpose.h>

namespace humanpose {
enum PinDataTypes
{
  HumanPose,
};
using InputData = std::variant<libvrm::HumanPose>;

struct GraphPin
{
  int Id;
  PinDataTypes DataType;

  GraphPin(int id, PinDataTypes dataType)
    : Id(id)
    , DataType(dataType)
  {
  }
};

struct Input
{
  std::string Name;
  GraphPin Pin;

  Input(std::string_view name, GraphPin pin)
    : Name(name)
    , Pin(pin)
  {
  }
};

struct Output
{
  std::string Name;
  GraphPin Pin;

  Output(std::string_view name, GraphPin pin)
    : Name(name)
    , Pin(pin)
  {
  }
  InputData Value;
};

struct GraphNodeBase
{
  int Id;
  std::string Prefix;
  std::string Name;
  std::vector<Input> Inputs;
  std::vector<Output> Outputs;
  float NodeWidth = 200.f;

  using InputNodes = std::span<InputData>;
  std::function<void(InputNodes)> Pull;

  GraphNodeBase(int id, std::string_view name)
    : Id(id)
    , Name(name)
  {
  }
  virtual ~GraphNodeBase() {}
  void Draw();
  virtual void TimeUpdate(libvrm::Time time) {}
  virtual void PullData(InputNodes inputs) {}
  virtual void DrawContent() {}
};
}
