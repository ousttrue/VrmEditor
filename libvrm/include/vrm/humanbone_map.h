#include "bvh.h"
#include "humanbones.h"
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace libvrm {
namespace vrm {

struct HumanBoneMap
{
  std::unordered_map<std::string, libvrm::vrm::HumanBones> NameBoneMap;
  std::unordered_set<std::string> NoBoneList;

  void Add(std::string_view joint_name, std::string_view bone_name)
  {
    if (auto bone = libvrm::vrm::HumanBoneFromName(
          bone_name, libvrm::vrm::VrmVersion::_1_0)) {
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

}
}
