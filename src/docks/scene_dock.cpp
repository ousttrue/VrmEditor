#include "scene_dock.h"
#include <imgui.h>
#include <memory>
#include <vrm/gizmo.h>
#include <vrm/mesh.h>
#include <vrm/node.h>

std::shared_ptr<TreeContext>
SceneDock::CreateTree(const AddDockFunc& addDock,
                      std::string_view title,
                      const std::shared_ptr<gltf::Scene>& scene,
                      float indent)
{
  auto context = std::make_shared<TreeContext>();

  auto enter = [scene, context](const std::shared_ptr<gltf::Node>& node) {
    static ImGuiTreeNodeFlags base_flags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
      ImGuiTreeNodeFlags_SpanAvailWidth;
    ImGuiTreeNodeFlags node_flags = base_flags;
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);

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

    bool node_open =
      ImGui::TreeNodeEx(&*node, node_flags, "%s", node->Label(*scene).c_str());

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
      int window_flags = 0;
      std::shared_ptr<gltf::Node> showSelected;
      if (auto selected = context->selected.lock()) {
        {
          if (auto mesh_index = selected->Mesh) {
            showSelected = selected;
            window_flags |= ImGuiWindowFlags_NoScrollbar;
          }
        }
      }

      if (ImGui::Begin(title, p_open, window_flags)) {
        auto size = ImGui::GetContentRegionAvail();
        context->selected = context->new_selected;

        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, indent);
        if (showSelected) {
          if (ImGui::BeginChild("##scene-tree",
                                { size.x, size.y / 2 },
                                true,
                                ImGuiWindowFlags_None)) {

            scene->Traverse(enter, leave);
          }
          ImGui::EndChild();

          if (ImGui::BeginChild("##scene-selected",
                                { size.x, size.y / 2 },
                                true,
                                ImGuiWindowFlags_None)) {
            ImGui::Text("%s", showSelected->Name.c_str());
            if (auto mesh_index = showSelected->Mesh) {
              auto mesh = scene->m_meshes[*mesh_index];
              auto instance = showSelected->Instance;
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
          ImGui::EndChild();
        } else {
          scene->Traverse(enter, leave);
        }
        ImGui::PopStyleVar();
      }
      ImGui::End();
      ImGui::PopStyleVar();
    }));

  return context;
}
