#pragma once
#include "gui.h"
#include "scene_preview.h"
#include <glr/rendering_env.h>
#include <imgui.h>
#include <vrm/runtime_scene.h>

class ViewDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     std::string_view title,
                     const std::shared_ptr<libvrm::RuntimeScene>& scene,
                     const std::shared_ptr<glr::RenderingEnv>& env,
                     const std::shared_ptr<grapho::OrbitView>& view,
                     const std::shared_ptr<glr::ViewSettings>& settings,
                     const std::shared_ptr<SceneNodeSelection>& selection)
  {
    auto preview = ScenePreview::Create(scene, env, view, settings, selection);

    addDock(grapho::imgui::Dock(
      title, [preview, scene, settings](const char* title, bool* p_open) {
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
                          const std::shared_ptr<libvrm::GltfRoot>& table,
                          const std::shared_ptr<glr::RenderingEnv>& env,
                          const std::shared_ptr<grapho::OrbitView>& view,
                          const std::shared_ptr<glr::ViewSettings>& settings,
                          const std::shared_ptr<SceneNodeSelection>& selection)
  {
    auto preview = ScenePreview::Create(table, env, view, settings, selection);

    addDock(grapho::imgui::Dock(
      title, [preview, table, settings](const char* title, bool* p_open) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
        if (ImGui::Begin(title,
                         p_open,
                         ImGuiWindowFlags_NoScrollbar |
                           ImGuiWindowFlags_NoScrollWithMouse)) {
          preview->ShowFullWindow(table->m_title.c_str(), settings->Color);
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
    addDock(grapho::imgui::Dock(title, [env, settings, view]() {
      if (ImGui::CollapsingHeader("View")) {
        ImGui::Checkbox("Mesh", &settings->ShowMesh);
        ImGui::SameLine();
        ImGui::Checkbox("Shadow", &settings->ShowShadow);
        ImGui::SameLine();
        ImGui::Checkbox("Gizmo", &settings->ShowLine);
        ImGui::SameLine();
        ImGui::Checkbox("Bone", &settings->ShowCuber);

        ImGui::ColorEdit4("Clear color", &env->ClearColor.x);
      }

      if (ImGui::CollapsingHeader("Camera")) {
        ImGui::SliderFloat("Near", &view->NearZ, 0.001f, 1000.0f);
        ImGui::SliderFloat("Far", &view->FarZ, 0.001f, 1000.0f);
        // TODO: reset position
      }

      if (ImGui::CollapsingHeader("SpringBone")) {
        // 60FPS
        ImGui::Checkbox("Spring", &settings->EnableSpring);
        if (settings->EnableSpring) {
        } else {
          if (ImGui::Button("Spring step")) {
            settings->NextSpringDelta = libvrm::Time(1.0 / 60);
          }
        }
      }
      if (settings->EnableSpring) {
        settings->NextSpringDelta = libvrm::Time(1.0 / 60);
      }

      if (ImGui::CollapsingHeader("Pbr Env")) {
        ImGui::Checkbox("Skybox", &settings->Skybox);
      }
    }));
  }
};
