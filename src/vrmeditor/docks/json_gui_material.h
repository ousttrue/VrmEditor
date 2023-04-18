#pragma once
#include "json_gui.h"
#include <imgui.h>

inline ShowGui
ShowSelected_materials()
{
  return []() {
    //
    ImGui::TextUnformatted("materials");
  };
}
