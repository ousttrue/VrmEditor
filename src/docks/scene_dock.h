#pragma once
#include "gui.h"
#include "treecontext.h"
#include <imgui.h>
#include <memory>
#include <vrm/gizmo.h>
#include <vrm/mesh.h>
#include <vrm/node.h>
#include <vrm/scene.h>

class SceneDock
{
public:
  static std::shared_ptr<TreeContext> CreateTree(
    const AddDockFunc& addDock,
    std::string_view title,
    const std::shared_ptr<gltf::Scene>& scene,
    float indent)
  {
    auto context = std::make_shared<TreeContext>();

    auto enter = [scene, context](const std::shared_ptr<gltf::Node>& node) {
      ImGui::SetNextItemOpen(true, ImGuiCond_Once);
      static ImGuiTreeNodeFlags base_flags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanAvailWidth;
      ImGuiTreeNodeFlags node_flags = base_flags;

      if (node->Children.empty()) {
        node_flags |=
          ImGuiTreeNodeFlags_Leaf |
          ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
      }
      if (context->selected.lock() == node) {
        node_flags |= ImGuiTreeNodeFlags_Selected;
      }

      bool hasRotation = node->InitialTransform.HasRotation();
      if (hasRotation) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
      }

      bool node_open = ImGui::TreeNodeEx(
        &*node, node_flags, "%s", node->Label(*scene).c_str());

      if (hasRotation) {
        ImGui::PopStyleColor();
      }

      if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        context->new_selected = node;
      }

      return node->Children.size() && node_open;
    };
    auto leave = []() { ImGui::TreePop(); };

    addDock(Dock(
      title,
      [scene, enter, leave, context, indent](const char* title, bool* p_open) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
        if (ImGui::Begin(title, p_open, ImGuiWindowFlags_NoScrollbar)) {
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
            if (auto selected = context->selected.lock()) {
              ImGui::Text("%s", selected->Name.c_str());
              if (auto mesh_index = selected->Mesh) {
                auto mesh = scene->m_meshes[*mesh_index];
                auto instance = selected->Instance;
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

    return context;
  }
};
