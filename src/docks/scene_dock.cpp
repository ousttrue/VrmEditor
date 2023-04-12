#include "scene_dock.h"
#include <imgui.h>
#include <memory>
#include <vrm/gizmo.h>
#include <vrm/mesh.h>
#include <vrm/node.h>
#include <vrm/vrm0.h>

const std::string
Label(const gltf::Scene& scene, const std::shared_ptr<gltf::Node>& node)
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
    // HumanBone = static_cast<vrm::HumanBones>(i);
    switch (humanoid->HumanBone) {
      case vrm::HumanBones::hips:
        ss << " ";
        break;
      case vrm::HumanBones::head:
        ss << "󱍞 ";
        break;
      case vrm::HumanBones::leftEye:
      case vrm::HumanBones::rightEye:
        ss << " ";
        break;
      case vrm::HumanBones::leftHand:
      case vrm::HumanBones::leftThumbMetacarpal:
      case vrm::HumanBones::leftThumbProximal:
      case vrm::HumanBones::leftThumbDistal:
      case vrm::HumanBones::leftIndexProximal:
      case vrm::HumanBones::leftIndexIntermediate:
      case vrm::HumanBones::leftIndexDistal:
      case vrm::HumanBones::leftMiddleProximal:
      case vrm::HumanBones::leftMiddleIntermediate:
      case vrm::HumanBones::leftMiddleDistal:
      case vrm::HumanBones::leftRingProximal:
      case vrm::HumanBones::leftRingIntermediate:
      case vrm::HumanBones::leftRingDistal:
      case vrm::HumanBones::leftLittleProximal:
      case vrm::HumanBones::leftLittleIntermediate:
      case vrm::HumanBones::leftLittleDistal:
        ss << "󰹆 ";
        break;
      case vrm::HumanBones::rightHand:
      case vrm::HumanBones::rightThumbMetacarpal:
      case vrm::HumanBones::rightThumbProximal:
      case vrm::HumanBones::rightThumbDistal:
      case vrm::HumanBones::rightIndexProximal:
      case vrm::HumanBones::rightIndexIntermediate:
      case vrm::HumanBones::rightIndexDistal:
      case vrm::HumanBones::rightMiddleProximal:
      case vrm::HumanBones::rightMiddleIntermediate:
      case vrm::HumanBones::rightMiddleDistal:
      case vrm::HumanBones::rightRingProximal:
      case vrm::HumanBones::rightRingIntermediate:
      case vrm::HumanBones::rightRingDistal:
      case vrm::HumanBones::rightLittleProximal:
      case vrm::HumanBones::rightLittleIntermediate:
      case vrm::HumanBones::rightLittleDistal:
        ss << "󰹇 ";
        break;
      case vrm::HumanBones::leftFoot:
      case vrm::HumanBones::leftToes:
      case vrm::HumanBones::rightFoot:
      case vrm::HumanBones::rightToes:
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
