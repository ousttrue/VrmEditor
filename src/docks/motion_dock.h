#pragma once
#include "gui.h"
// #include <filesystem>
// #include <functional>
// #include <list>
// #include <memory>
// #include <vrm/bvh.h>
// #include <vrm/bvhsolver.h>
// #include <vrm/humanbones.h>
// #include <vrm/humanpose.h>
// #include <vrm/timeline.h>
#include <vrm/bvhsource.h>


struct GraphPin
{
  std::string Name;
};

struct GraphNode
{
  std::string Prefix;
  std::string Name;
  std::vector<GraphPin> Outputs;
  std::vector<GraphPin> Inputs;
};

class MotionDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     const std::shared_ptr<bvh::MotionSource>& motion_source);
};
