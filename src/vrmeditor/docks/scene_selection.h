#pragma once
#include <vrm/node.h>

struct SceneNodeSelection
{
  std::weak_ptr<libvrm::Node> selected;
  std::weak_ptr<libvrm::Node> new_selected;
};
