#include "gui.h"
#include <functional>
#include <list>
#include <string>
#include <string_view>
#include <vrm/humanpose.h>
#include <vrm/timeline.h>

using HumanPoseFunc = std::function<void(const libvrm::vrm::HumanPose& pose)>;

enum PinDataTypes
{
  None,
  HumanPose,
};

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
};

struct GraphNode
{
  int Id;
  std::string Prefix;
  std::string Name;
  std::vector<Input> Inputs;
  std::vector<Output> Outputs;

  GraphNode(int id, std::string_view name)
    : Id(id)
    , Name(name)
  {
  }

  void Draw();
};

struct Link
{
  int Id;
  int Start;
  int End;

  void Draw();
};

struct PinNameWithType
{
  std::string Name;
  PinDataTypes Type;
};

struct HumanPoseStream
{
  int m_nextNodeId = 1;
  int m_nextPinId = 1;
  std::list<std::shared_ptr<GraphNode>> m_nodes;
  int m_nextLinkId = 1;
  std::list<Link> Links;
  std::list<HumanPoseFunc> HumanPoseChanged;

  HumanPoseStream();
  ~HumanPoseStream();
  void LoadIni(std::string_view ini);
  std::string Save();

  std::shared_ptr<GraphNode> CreateNode(
    std::string_view name,
    std::string_view prefix,
    std::span<const PinNameWithType> inputs,
    std::span<const PinNameWithType> outputs);
  std::tuple<std::shared_ptr<GraphNode>, PinDataTypes> FindNodeFromOutput(
    int start) const;
  std::tuple<std::shared_ptr<GraphNode>, PinDataTypes> FindNodeFromInput(
    int start) const;
  void TryCreateLink(int start, int end);
  void TryRemoveLink(int link_id);

  void SetHumanPose(const libvrm::vrm::HumanPose& pose)
  {
    for (auto& callback : HumanPoseChanged) {
      callback(pose);
    }
  }

  void CreateDock(const AddDockFunc& addDock);
  void Update(libvrm::Time time);
};
