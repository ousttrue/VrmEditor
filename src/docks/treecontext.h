#pragma once
#include <vrm/node.h>
#include <memory>

struct TreeContext
{
  std::weak_ptr<gltf::Node> selected;
  std::weak_ptr<gltf::Node> new_selected;
};
