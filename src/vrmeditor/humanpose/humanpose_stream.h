#pragma once
#include "docks/dockspace.h"
#include "graphnode_base.h"
#include <list>
#include <vrm/bvh/humanbone_map.h>

class Cuber;
class UdpReceiver;

namespace humanpose {
using HumanPoseFunc = std::function<bool(const libvrm::HumanPose& pose)>;

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

  std::list<std::shared_ptr<libvrm::HumanBoneMap>> m_humanBoneMapList;
  std::shared_ptr<libvrm::HumanBoneMap> AddHumanBoneMap()
  {
    auto ptr = std::make_shared<libvrm::HumanBoneMap>();
    m_humanBoneMapList.push_back(ptr);
    return ptr;
  }
  std::shared_ptr<libvrm::HumanBoneMap> FindHumanBoneMap(
    const libvrm::bvh::Bvh& bvh) const
  {
    for (auto& map : m_humanBoneMapList) {
      if (map->Match(bvh)) {
        return map;
      }
    }
    return {};
  }

private:
  HumanPoseStream();

public:
  ~HumanPoseStream();
  HumanPoseStream(const HumanPoseStream&) = delete;
  HumanPoseStream& operator=(const HumanPoseStream&) = delete;
  static HumanPoseStream& Instance()
  {
    static HumanPoseStream s_instance;
    return s_instance;
  }

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

  std::tuple<std::shared_ptr<GraphNodeBase>, size_t> FindNodeFromOutput(
    int start) const;
  std::tuple<std::shared_ptr<GraphNodeBase>, size_t> FindNodeFromInput(
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

  bool LoadMotion(const std::filesystem::path& path);
  bool LoadVrmPose(const std::string &json);
  void ShowGui();
  void Update(libvrm::Time time, std::shared_ptr<GraphNodeBase> node = {});
};

} // namespace
