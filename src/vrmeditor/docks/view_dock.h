#pragma once
#include "glr/scene_preview.h"
#include "gui.h"
#include <imgui.h>

class ViewDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     std::string_view title,
                     const std::shared_ptr<libvrm::gltf::Scene>& scene,
                     const std::shared_ptr<libvrm::gltf::SceneContext>& context,
                     const std::shared_ptr<glr::RenderingEnv>& env,
                     const std::shared_ptr<grapho::OrbitView>& view,
                     const std::shared_ptr<glr::ViewSettings>& settings)
  {
    auto preview =
      std::make_shared<glr::ScenePreview>(scene, context, env, view, settings);

    addDock(
      Dock(title, [preview, scene, settings](const char* title, bool* p_open) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
        if (ImGui::Begin(title,
                         p_open,
                         ImGuiWindowFlags_NoScrollbar |
                           ImGuiWindowFlags_NoScrollWithMouse)) {
          preview->ShowFullWindow(scene->m_title.c_str(), settings->Color);
        }
        ImGui::End();
        ImGui::PopStyleVar();
      }));
  }

  static void CreateSetting(const AddDockFunc& addDock,
                            std::string_view title,
                            const std::shared_ptr<glr::RenderingEnv>& env,
                            const std::shared_ptr<grapho::OrbitView>& view,
                            const std::shared_ptr<glr::ViewSettings>& settings)
  {
    addDock(Dock(title, [env, settings]() {
      ImGui::Checkbox("Mesh", &settings->ShowMesh);
      ImGui::SameLine();
      ImGui::Checkbox("Shadow", &settings->ShowShadow);
      ImGui::SameLine();
      ImGui::Checkbox("Gizmo", &settings->ShowLine);
      ImGui::SameLine();
      ImGui::Checkbox("Bone", &settings->ShowCuber);

      // 60FPS
      ImGui::Checkbox("spring", &settings->EnableSpring);
      if (settings->EnableSpring) {
        settings->NextSpringDelta = libvrm::Time(1.0 / 60);
      } else {
        if (ImGui::Button("spring step")) {
          settings->NextSpringDelta = libvrm::Time(1.0 / 60);
        }
      }

      ImGui::ColorEdit4("clear color", env->ClearColor);

      // camera
      // near, far
      // reset
    }));
  }
};
