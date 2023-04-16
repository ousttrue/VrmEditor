#pragma once
#include "gui.h"
#include <functional>
#include <list>
#include <string>
#include <string_view>
#include <vrm/humanbone_map.h>
#include <vrm/humanpose.h>
#include <vrm/timeline.h>

class Cuber;
class UdpReceiver;

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

struct GraphNodeBase
{
  int Id;
  std::string Prefix;
  std::string Name;
  std::vector<Input> Inputs;
  std::vector<Output> Outputs;
  float NodeWidth = 200.f;

  GraphNodeBase(int id, std::string_view name)
    : Id(id)
    , Name(name)
  {
  }
  virtual ~GraphNodeBase() {}
  void Draw();
  virtual void Update(libvrm::Time time) {}
  virtual void DrawContent() {}
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
  std::list<std::shared_ptr<GraphNodeBase>> m_nodes;
  int m_nextLinkId = 1;
  std::list<std::shared_ptr<Link>> Links;
  std::list<HumanPoseFunc> HumanPoseChanged;

  std::list<std::shared_ptr<libvrm::vrm::HumanBoneMap>> m_humanBoneMapList;
  std::shared_ptr<libvrm::vrm::HumanBoneMap> AddHumanBoneMap()
  {
    auto ptr = std::make_shared<libvrm::vrm::HumanBoneMap>();
    m_humanBoneMapList.push_back(ptr);
    return ptr;
  }
  std::shared_ptr<libvrm::vrm::HumanBoneMap> FindHumanBoneMap(
    const libvrm::bvh::Bvh& bvh) const
  {
    for (auto& map : m_humanBoneMapList) {
      if (map->Match(bvh)) {
        return map;
      }
    }
    return {};
  }

  HumanPoseStream();
  ~HumanPoseStream();
  void LoadIni(std::string_view ini);
  std::string Save();

  template<typename T>
  std::shared_ptr<T> CreateNode(std::string_view name,
                                std::string_view prefix,
                                std::span<const PinNameWithType> inputs,
                                std::span<const PinNameWithType> outputs)
  {
    auto ptr = std::make_shared<T>(m_nextNodeId++, name);
    ptr->Prefix = prefix;
    for (auto [pinName, pinDataType] : inputs) {
      ptr->Inputs.push_back({ pinName, { m_nextPinId++, pinDataType } });
    }
    for (auto [pinName, pinDataType] : outputs) {
      ptr->Outputs.push_back({ pinName, { m_nextPinId++, pinDataType } });
    }
    m_nodes.push_back(ptr);
    return ptr;
  }

  std::tuple<std::shared_ptr<GraphNodeBase>, PinDataTypes> FindNodeFromOutput(
    int start) const;
  std::tuple<std::shared_ptr<GraphNodeBase>, PinDataTypes> FindNodeFromInput(
    int end) const;
  void TryCreateLink(int start, int end);
  void TryRemoveLink(int link_id);
  std::shared_ptr<Link> FindLinkFromEnd(int end)
  {
    for (auto& link : Links) {
      if (link->End == end) {
        return link;
      }
    }
    return {};
  }

  void SetHumanPose(const libvrm::vrm::HumanPose& pose)
  {
    for (auto& callback : HumanPoseChanged) {
      callback(pose);
    }
  }

  bool LoadMotion(const std::filesystem::path& path);

  void CreateDock(const AddDockFunc& addDock);
  void Update(libvrm::Time time, std::shared_ptr<GraphNodeBase> node = {});
};
