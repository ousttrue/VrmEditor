#pragma once
#include "gui.h"
#include "imhumanoid.h"

class HumanoidDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     std::string_view body_title,
                     std::string_view finger_title,
                     const std::shared_ptr<libvrm::GltfRoot>& scene)
  {
    auto imHumanoid = std::make_shared<ImHumanoid>();

    addDock({
      .Name = { body_title.begin(), body_title.end() },
      .OnShow =
        [scene, imHumanoid]() {
          //
          imHumanoid->ShowBody(*scene);
        },
      .StyleVars = { { ImGuiStyleVar_WindowPadding, { 0, 0 } } },
    });
    addDock({
      .Name = { finger_title.begin(), finger_title.end() },
      .OnShow =
        [scene, imHumanoid]() {
          //
          imHumanoid->ShowFingers(*scene);
        },
      .StyleVars = { { ImGuiStyleVar_WindowPadding, { 0, 0 } } },
    });
  }
};
