#pragma once
#include "glr/scene_preview.h"
#include "gui.h"
#include <imgui.h>
#include <vrm/runtimescene/scene.h>

class ViewDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     std::string_view title,
                     const std::shared_ptr<runtimescene::RuntimeScene>& scene,
                     const std::shared_ptr<glr::RenderingEnv>& env,
                     const std::shared_ptr<grapho::OrbitView>& view,
                     const std::shared_ptr<glr::ViewSettings>& settings,
                     const std::shared_ptr<SceneNodeSelection>& selection)
  {
    auto preview = std::make_shared<glr::ScenePreview>(
      scene, env, view, settings, selection, false);

    addDock(
      Dock(title, [preview, scene, settings](const char* title, bool* p_open) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
        if (ImGui::Begin(title,
                         p_open,
                         ImGuiWindowFlags_NoScrollbar |
                           ImGuiWindowFlags_NoScrollWithMouse)) {
          preview->ShowFullWindow(scene->m_table->m_title.c_str(),
                                  settings->Color);
        }
        ImGui::End();
        ImGui::PopStyleVar();
      }));
  }

  static void CreateTPose(const AddDockFunc& addDock,
                          std::string_view title,
                          const std::shared_ptr<libvrm::gltf::Scene>& table,
                          const std::shared_ptr<glr::RenderingEnv>& env,
                          const std::shared_ptr<glr::ViewSettings>& settings,
                          const std::shared_ptr<SceneNodeSelection>& selection)
  {
    auto view = std::make_shared<grapho::OrbitView>();
    auto scene = std::make_shared<runtimescene::RuntimeScene>(table);
    auto preview = std::make_shared<glr::ScenePreview>(
      scene, env, view, settings, selection, true);

    addDock(
      Dock(title, [preview, scene, settings](const char* title, bool* p_open) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
        if (ImGui::Begin(title,
                         p_open,
                         ImGuiWindowFlags_NoScrollbar |
                           ImGuiWindowFlags_NoScrollWithMouse)) {
          preview->ShowFullWindow(scene->m_table->m_title.c_str(),
                                  settings->Color);
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
