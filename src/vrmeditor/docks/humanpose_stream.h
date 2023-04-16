#include "gui.h"
#include <functional>
#include <list>
#include <string>
#include <string_view>
#include <vrm/humanpose.h>
#include <vrm/timeline.h>

using HumanPoseFunc = std::function<void(const libvrm::vrm::HumanPose& pose)>;

struct HumanPoseStream
{
  struct HumanPoseStreamImpl* m_impl = nullptr;

public:
  HumanPoseStream();
  ~HumanPoseStream();

  void Load(std::string_view ini);
  std::string Save();

  std::list<HumanPoseFunc> HumanPoseChanged;
  void SetHumanPose(const libvrm::vrm::HumanPose& pose)
  {
    for (auto& callback : HumanPoseChanged) {
      callback(pose);
    }
  }

  void CreateDock(const AddDockFunc& addDock);

  void Update(libvrm::Time time);
};
