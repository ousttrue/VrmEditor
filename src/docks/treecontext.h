#pragma once
#include <vrm/node.h>

struct TreeContext
{
  gltf::Node* selected = nullptr;
  gltf::Node* new_selected = nullptr;
};
