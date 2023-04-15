#include <functional>
#include <list>
#include <vrm/humanpose.h>
#include <vrm/timeline.h>

using HumanPoseFunc = std::function<void(const libvrm::vrm::HumanPose& pose)>;

// Timeline
//
// clip----------+
//               |
// udp_receiver--switch -> pose
//               |
// tpose --------+
//
struct HumanPoseStream
{
  std::list<HumanPoseFunc> HumanPoseChanged;
  void SetHumanPose(const libvrm::vrm::HumanPose& pose)
  {
    for (auto& callback : HumanPoseChanged) {
      callback(pose);
    }
  }

  void Update(libvrm::Time time);
};
