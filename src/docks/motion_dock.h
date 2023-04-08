#pragma once
#include "gui.h"

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
                     std::string_view title,
                     const std::shared_ptr<Cuber>& cuber);
};
