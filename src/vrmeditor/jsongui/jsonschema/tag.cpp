#include "tag.h"
#include <imgui.h>

ShowTagFunc
MaterialTag(const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            const gltfjson::tree::NodePtr& item)
{
  return []() { ImGui::SmallButton("unlit"); };
}
