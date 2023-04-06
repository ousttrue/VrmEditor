#pragma once
#include "gl3renderer.h"
#include "gui.h"
#include "imhumanoid.h"
#include "rendertarget.h"
#include <cuber/gl3/GlLineRenderer.h>
#include <imgui.h>
#include <memory>
#include <vrm/gizmo.h>
#include <vrm/mesh.h>
#include <vrm/node.h>
#include <vrm/scene.h>
#include <vrm/vrm0.h>
#include <vrm/vrm1.h>

class SceneDock
{
  struct TreeContext
  {
    gltf::Node* selected = nullptr;
    gltf::Node* new_selected = nullptr;
  };

public:
  static void Create(const AddDockFunc& addDock,
                     const std::shared_ptr<gltf::Scene>& scene,
                     const std::shared_ptr<OrbitView>& view,
                     const std::shared_ptr<Timeline>& timeline,
                     const std::shared_ptr<Gl3Renderer>& gl3r,
                     float indent)
  {
    //
    // scene tree
    //
    auto context = std::make_shared<TreeContext>();

    auto enter = [scene, context](gltf::Node& node) {
      ImGui::SetNextItemOpen(true, ImGuiCond_Once);
      static ImGuiTreeNodeFlags base_flags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanAvailWidth;
      ImGuiTreeNodeFlags node_flags = base_flags;

      if (node.Children.empty()) {
        node_flags |=
          ImGuiTreeNodeFlags_Leaf |
          ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
      }
      if (context->selected == &node) {
        node_flags |= ImGuiTreeNodeFlags_Selected;
      }

      bool hasRotation = node.InitialTransform.HasRotation();
      if (hasRotation) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
      }

      bool node_open =
        ImGui::TreeNodeEx(&node, node_flags, "%s", node.Label(*scene).c_str());

      if (hasRotation) {
        ImGui::PopStyleColor();
      }

      if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        context->new_selected = &node;
      }

      return node.Children.size() && node_open;
    };
    auto leave = []() { ImGui::TreePop(); };

    addDock(Dock("scene", [scene, enter, leave, context, indent](bool* p_open) {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
      if (ImGui::Begin("scene", p_open, ImGuiWindowFlags_NoScrollbar)) {
        auto size = ImGui::GetContentRegionAvail();

        context->selected = context->new_selected;
        // ImGui::BeginGroup();
        if (ImGui::BeginChild("##scene-tree",
                              { size.x, size.y / 2 },
                              true,
                              ImGuiWindowFlags_None)) {

          ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, indent);
          scene->Traverse(enter, leave);
          ImGui::PopStyleVar();
        }
        ImGui::EndChild();
        // ImGui::EndGroup();

        // ImGui::BeginGroup();
        if (ImGui::BeginChild("##scene-selected",
                              { size.x, size.y / 2 },
                              true,
                              ImGuiWindowFlags_None)) {
          if (context->selected) {
            ImGui::Text("%s", context->selected->Name.c_str());
            if (auto mesh_index = context->selected->Mesh) {
              auto mesh = scene->m_meshes[*mesh_index];
              auto instance = context->selected->Instance;
              char morph_id[256];
              for (int i = 0; i < mesh->m_morphTargets.size(); ++i) {
                auto& morph = mesh->m_morphTargets[i];
                snprintf(morph_id,
                         sizeof(morph_id),
                         "[%d]%s##morph%d",
                         i,
                         morph->Name.c_str(),
                         i);
                ImGui::SliderFloat(morph_id, &instance->weights[i], 0, 1);
              }
            }
          }
        }
        ImGui::EndChild();
        // ImGui::EndGroup();
      }
      ImGui::End();
      ImGui::PopStyleVar();
    }));

    auto imHumanoid = std::make_shared<ImHumanoid>();

    addDock(Dock("human-body", [scene, imHumanoid](bool* p_open) {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
      if (ImGui::Begin("human-body", p_open)) {
        imHumanoid->ShowBody(*scene);
      }
      ImGui::End();
      ImGui::PopStyleVar();
    }));
    addDock(Dock("human-fingers", [scene, imHumanoid](bool* p_open) {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
      if (ImGui::Begin("human-fingers", p_open)) {
        imHumanoid->ShowFingers(*scene);
      }
      ImGui::End();
      ImGui::PopStyleVar();
    }));

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

    //
    // 3d view
    //
    auto rt = std::make_shared<RenderTarget>(view);
    rt->color[0] = 0.2f;
    rt->color[1] = 0.2f;
    rt->color[2] = 0.2f;
    rt->color[3] = 1.0f;

    rt->render = [timeline, scene, gl3r, selection = context](
                   const ViewProjection& camera) {
      gl3r->ClearRendertarget(camera);

      auto liner = std::make_shared<cuber::gl3::GlLineRenderer>();

      gltf::RenderFunc render =
        [gl3r, liner, &camera](const gltf::Mesh& mesh,
                               const gltf::MeshInstance& instance,
                               const float m[16]) {
          gl3r->Render(camera, mesh, instance, m);
        };
      scene->Render(timeline->CurrentTime, render);
      liner->Render(camera.projection, camera.view, gizmo::lines());

      // gizmo
      if (auto node = selection->selected) {
        // TODO: conflict mouse event(left) with ImageButton
        DirectX::XMFLOAT4X4 m;
        DirectX::XMStoreFloat4x4(&m, node->WorldMatrix());
        ImGuizmo::GetContext().mAllowActiveHoverItem = true;
        if (ImGuizmo::Manipulate(camera.view,
                                 camera.projection,
                                 ImGuizmo::TRANSLATE | ImGuizmo::ROTATE,
                                 ImGuizmo::LOCAL,
                                 (float*)&m,
                                 nullptr,
                                 nullptr,
                                 nullptr,
                                 nullptr)) {
          // decompose feedback
          node->SetWorldMatrix(DirectX::XMLoadFloat4x4(&m));
        }
        ImGuizmo::GetContext().mAllowActiveHoverItem = false;
      }
    };

    addDock(Dock("view", [rt, scene](bool* p_open) {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
      if (ImGui::Begin("render target",
                       p_open,
                       ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoScrollWithMouse)) {
        auto pos = ImGui::GetWindowPos();
        pos.y += ImGui::GetFrameHeight();
        auto size = ImGui::GetContentRegionAvail();
        rt->show_fbo(pos.x, pos.y, size.x, size.y);
      }

      ImGui::End();
      ImGui::PopStyleVar();
    }));
  }
};
