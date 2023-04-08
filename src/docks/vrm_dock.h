#pragma once
#include "gui.h"
#include <vrm/scene.h>
#include <vrm/vrm0.h>
#include <vrm/vrm1.h>

class VrmDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     const std::shared_ptr<gltf::Scene>& scene)
  {
    addDock(Dock("vrm", [scene]() {
      if (auto vrm = scene->m_vrm0) {
        ImGui::Text("%s", "vrm-0.x");
        for (auto expression : vrm->m_expressions) {
          ImGui::SliderFloat(
            expression->label.c_str(), &expression->weight, 0, 1);
        }
      }
      if (auto vrm = scene->m_vrm1) {
        ImGui::Text("%s", "vrm-1.0");
        // for (auto expression : vrm->m_expressions) {
        //   ImGui::SliderFloat(
        //     expression->label.c_str(), &expression->weight, 0, 1);
        // }
      }
    }));
  }
};
