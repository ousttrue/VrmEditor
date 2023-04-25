#include "scene_gui.h"
#include <imgui.h>
#include <vrm/mesh.h>
#include <vrm/scene.h>

SceneGui::SceneGui(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                   float indent)
  : m_scene(scene)
  , m_indent(indent)
  , Context(new libvrm::gltf::SceneContext)
{
}

// [gltf]
// textures
// materials
// meshes
// nodes/node-hierarchy/select
// animations
//
// [vrm]
// meta
// humanoid
// expression
// lookat
// firstperson
// spring
// constraint
//
void
SceneGui::Show(const char* title, bool* p_open)
{
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
  int window_flags = 0;
  std::shared_ptr<libvrm::gltf::Node> showSelected;
  if (auto selected = Context->selected.lock()) {
    {
      if (auto mesh_index = selected->Mesh) {
        showSelected = selected;
        window_flags |= ImGuiWindowFlags_NoScrollbar;
      }
    }
  }

  if (ImGui::Begin(title, p_open, window_flags)) {
    auto size = ImGui::GetContentRegionAvail();
    Context->selected = Context->new_selected;

    // 60FPS
    ImGui::Checkbox("spring", &m_enableSpring);
    if (m_enableSpring) {
      m_scene->m_nextSpringDelta = libvrm::Time(1.0 / 60);
    } else {
      if (ImGui::Button("spring step")) {
        m_scene->m_nextSpringDelta = libvrm::Time(1.0 / 60);
      }
    }

    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, m_indent);
    if (showSelected) {
      if (ImGui::BeginChild("##scene-tree",
                            { size.x, size.y / 2 },
                            true,
                            ImGuiWindowFlags_None)) {

        m_scene->Traverse(
          [this](const std::shared_ptr<libvrm::gltf::Node>& node) {
            return Enter(node);
          },
          [this]() { Leave(); });
      }
      ImGui::EndChild();

      if (ImGui::BeginChild("##scene-selected",
                            { size.x, size.y / 2 },
                            true,
                            ImGuiWindowFlags_None)) {
        ImGui::Text("%s", showSelected->Name.c_str());
        if (auto mesh_index = showSelected->Mesh) {
          auto mesh = m_scene->m_meshes[*mesh_index];
          auto meshInstance = showSelected->MeshInstance;
          char morph_id[256];
          for (int i = 0; i < mesh->m_morphTargets.size(); ++i) {
            auto& morph = mesh->m_morphTargets[i];
            snprintf(morph_id,
                     sizeof(morph_id),
                     "[%d]%s##morph%d",
                     i,
                     morph->Name.c_str(),
                     i);
            ImGui::SliderFloat(morph_id, &meshInstance->weights[i], 0, 1);
          }
        }
      }
      ImGui::EndChild();
    } else {
      m_scene->Traverse(
        [this](const std::shared_ptr<libvrm::gltf::Node>& node) {
          return Enter(node);
        },
        [this]() { Leave(); });
    }
    ImGui::PopStyleVar();
  }
  ImGui::End();
  ImGui::PopStyleVar();
}

bool
SceneGui::Enter(const std::shared_ptr<libvrm::gltf::Node>& node)
{
  static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                         ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                         ImGuiTreeNodeFlags_SpanAvailWidth;
  ImGuiTreeNodeFlags node_flags = base_flags;
  ImGui::SetNextItemOpen(true, ImGuiCond_Once);

  if (node->Children.empty()) {
    node_flags |=
      ImGuiTreeNodeFlags_Leaf |
      ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
  }
  if (Context->selected.lock() == node) {
    node_flags |= ImGuiTreeNodeFlags_Selected;
  }

  bool hasRotation = node->InitialTransform.HasRotation();
  int push = 0;
  if (node->Constraint) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
    ++push;
  } else if (hasRotation) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
    ++push;
  }

  bool node_open =
    ImGui::TreeNodeEx(&*node, node_flags, "%s", Label(*m_scene, node).c_str());

  ImGui::PopStyleColor(push);

  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
    Context->new_selected = node;
  }

  return node->Children.size() && node_open;
}

void
SceneGui::Leave()
{
  ImGui::TreePop();
}

const std::string&
SceneGui::Label(const libvrm::gltf::Scene& scene,
                const std::shared_ptr<libvrm::gltf::Node>& node)
{
  auto found = m_map.find(node);
  if (found != m_map.end()) {
    return found->second;
  }

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
  // spring
  for (auto& solver : scene.m_springSolvers) {
    for (auto& joint : solver->Joints) {
      auto joint_node = joint.Head;
      if (joint_node == node) {
        ss << "󰚟 ";
        break;
      }
    }
  }
  // collider
  for (auto& collider : scene.m_springColliders) {
    if (auto colliderNode = collider->Node.lock()) {
      if (colliderNode == node) {
        ss << "󱥔 ";
        break;
      }
    }
  }

  ss << node->Name;

  auto inserted = m_map.insert({ node, ss.str() });
  return inserted.first->second;
}
