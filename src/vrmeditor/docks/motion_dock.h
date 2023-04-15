#pragma once
#include "gui.h"
#include "treecontext.h"
#include <functional>

class MotionDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     std::string_view title,
                     const std::shared_ptr<class Cuber>& cuber,
                     const std::shared_ptr<TreeContext>& context,
                     const std::function<void()>& startUdp,
                     const std::function<void()>& stopUdp);
};
