#include "scene_dock.h"
#include <imgui.h>
#include <memory>
#include <vrm/gizmo.h>
#include <vrm/mesh.h>
#include <vrm/node.h>
#include <vrm/vrm0.h>

const std::string
Label(const libvrm::gltf::Scene& scene,
      const std::shared_ptr<libvrm::gltf::Node>& node)
{
  std::stringstream ss;
  if (auto mesh_index = node->Mesh) {
    ss << "󰕣 ";
    auto mesh = scene.m_meshes[*mesh_index];
    if (mesh->m_morphTargets.size()) {
      ss << " ";
    }
  }

  if (auto humanoid = node->Humanoid) {
    // HumanBone = static_cast<libvrm::vrm::HumanBones>(i);
    switch (humanoid->HumanBone) {
      case libvrm::vrm::HumanBones::hips:
        ss << " ";
        break;
      case libvrm::vrm::HumanBones::head:
        ss << "󱍞 ";
        break;
      case libvrm::vrm::HumanBones::leftEye:
      case libvrm::vrm::HumanBones::rightEye:
        ss << " ";
        break;
      case libvrm::vrm::HumanBones::leftHand:
      case libvrm::vrm::HumanBones::leftThumbMetacarpal:
      case libvrm::vrm::HumanBones::leftThumbProximal:
      case libvrm::vrm::HumanBones::leftThumbDistal:
      case libvrm::vrm::HumanBones::leftIndexProximal:
      case libvrm::vrm::HumanBones::leftIndexIntermediate:
      case libvrm::vrm::HumanBones::leftIndexDistal:
      case libvrm::vrm::HumanBones::leftMiddleProximal:
      case libvrm::vrm::HumanBones::leftMiddleIntermediate:
      case libvrm::vrm::HumanBones::leftMiddleDistal:
      case libvrm::vrm::HumanBones::leftRingProximal:
      case libvrm::vrm::HumanBones::leftRingIntermediate:
      case libvrm::vrm::HumanBones::leftRingDistal:
      case libvrm::vrm::HumanBones::leftLittleProximal:
      case libvrm::vrm::HumanBones::leftLittleIntermediate:
      case libvrm::vrm::HumanBones::leftLittleDistal:
        ss << "󰹆 ";
        break;
      case libvrm::vrm::HumanBones::rightHand:
      case libvrm::vrm::HumanBones::rightThumbMetacarpal:
      case libvrm::vrm::HumanBones::rightThumbProximal:
      case libvrm::vrm::HumanBones::rightThumbDistal:
      case libvrm::vrm::HumanBones::rightIndexProximal:
      case libvrm::vrm::HumanBones::rightIndexIntermediate:
      case libvrm::vrm::HumanBones::rightIndexDistal:
      case libvrm::vrm::HumanBones::rightMiddleProximal:
      case libvrm::vrm::HumanBones::rightMiddleIntermediate:
      case libvrm::vrm::HumanBones::rightMiddleDistal:
      case libvrm::vrm::HumanBones::rightRingProximal:
      case libvrm::vrm::HumanBones::rightRingIntermediate:
      case libvrm::vrm::HumanBones::rightRingDistal:
      case libvrm::vrm::HumanBones::rightLittleProximal:
      case libvrm::vrm::HumanBones::rightLittleIntermediate:
      case libvrm::vrm::HumanBones::rightLittleDistal:
        ss << "󰹇 ";
        break;
      case libvrm::vrm::HumanBones::leftFoot:
      case libvrm::vrm::HumanBones::leftToes:
      case libvrm::vrm::HumanBones::rightFoot:
      case libvrm::vrm::HumanBones::rightToes:
        ss << "󱗈 ";
        break;
      default:
        ss << "󰂹 ";
        break;
    }
  }

  // vrm0
  if (auto vrm = scene.m_vrm0) {
    // spring
    if (vrm->m_springs.size()) {
      for (auto& spring : vrm->m_springs) {
        for (auto joint : spring->bones) {
          auto joint_node = scene.m_nodes[joint];
          if (joint_node == node) {
            ss << "󰚟 ";
            break;
          }
        }
      }
    }
    // collider
    if (vrm->m_colliderGroups.size()) {
    }
  }

  ss << node->Name;
  return ss.str();
}

std::shared_ptr<TreeContext>
SceneDock::CreateTree(const AddDockFunc& addDock,
                      std::string_view title,
                      const std::shared_ptr<libvrm::gltf::Scene>& scene,
                      float indent)
{
  auto context = std::make_shared<TreeContext>();

  auto enter = [scene,
                context](const std::shared_ptr<libvrm::gltf::Node>& node) {
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
      ImGui::TreeNodeEx(&*node, node_flags, "%s", Label(*scene, node).c_str());

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
      std::shared_ptr<libvrm::gltf::Node> showSelected;
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
