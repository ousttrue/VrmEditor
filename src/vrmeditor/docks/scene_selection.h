#pragma once
#include <vrm/node.h>

struct SceneNodeSelection
{
  std::weak_ptr<libvrm::gltf::Node> selected;
  std::weak_ptr<libvrm::gltf::Node> new_selected;
};
