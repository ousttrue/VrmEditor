#pragma once
#include "bvh.h"
#include "../humanoid/humanbones.h"
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace libvrm {

struct HumanBoneMap
{
  std::unordered_map<std::string, libvrm::HumanBones> NameBoneMap;
  std::unordered_set<std::string> NoBoneList;

  void Add(std::string_view joint_name, std::string_view bone_name)
  {
    if (auto bone = libvrm::HumanBoneFromName(
          bone_name, libvrm::VrmVersion::_1_0)) {
      NameBoneMap[{ joint_name.begin(), joint_name.end() }] = *bone;
    } else {
      NoBoneList.insert({ joint_name.begin(), joint_name.end() });
    }
  }

  bool Match(const libvrm::bvh::Bvh& bvh) const
  {
    for (auto& joint : bvh.joints) {
      auto found = NameBoneMap.find(joint.name);
      if (found == NameBoneMap.end()) {
        if (NoBoneList.find(joint.name) != NoBoneList.end()) {
          // no bone joint
          continue;
        }
        return false;
      }
    }
    return true;
  }
};

} // namespace
