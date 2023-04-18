#pragma once
#include "json_gui.h"
#include <imgui.h>

inline ShowGui
ShowSelected_images()
{
  return []() {
    //
    ImGui::TextUnformatted("images");
  };
}
