#pragma once
#include "graphnode_base.h"

namespace humanpose {
struct PoseNode : public GraphNodeBase
{
  libvrm::HumanPosePayload Payload;

  PoseNode(int id, std::string_view name)
    : GraphNodeBase(id, name)
  {
    Payload.SetPose(libvrm::HumanPose::Initial());
  }

  void TimeUpdate(libvrm::Time time) override { Outputs[0].Value = Payload.GetPose(); }
};

}
