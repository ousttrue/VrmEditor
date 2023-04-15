#include "gui.h"
#include <functional>
#include <list>
#include <vrm/humanpose.h>
#include <vrm/timeline.h>

using HumanPoseFunc = std::function<void(const libvrm::vrm::HumanPose& pose)>;

struct HumanPoseStream
{
  struct HumanPoseStreamImpl* m_impl = nullptr;

public:
  HumanPoseStream();
  ~HumanPoseStream();

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
