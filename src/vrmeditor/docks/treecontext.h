#pragma once
#include <vrm/node.h>
#include <memory>

struct TreeContext
{
  std::weak_ptr<libvrm::gltf::Node> selected;
  std::weak_ptr<libvrm::gltf::Node> new_selected;
};
