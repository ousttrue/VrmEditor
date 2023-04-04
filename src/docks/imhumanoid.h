#pragma once
#include <vrm/humanoid.h>
#include <sstream>
#include <imgui.h>

class ImHumanoid
{
public:
  static void Show(const vrm::Humanoid &humanoid)
  {
    std::stringstream ss;
    for(int i=0; i<(int)vrm::HumanBones::VRM_BONE_COUNT; ++i)
    {
      ss.str("");
      if(auto node_index = humanoid[i])
      {
        ss << *node_index;
      }
      else{
        ss << "--";
      }
      ImGui::LabelText(vrm::HumanBonesNames[i], "%s", ss.str().c_str());
    }
  }
};
