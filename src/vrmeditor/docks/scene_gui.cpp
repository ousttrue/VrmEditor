#include <GL/glew.h>

#include "scene_gui.h"
#include "scene_gui_material.h"
#include "scene_selection.h"
#include "type_gui.h"
#include <glr/gl3renderer.h>
#include <grapho/gl3/Texture.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <vrm/material.h>
#include <vrm/mesh.h>
#include <vrm/runtimescene/mesh.h>
#include <vrm/runtimescene/scene.h>
#include <vrm/scene.h>
#include <vrm/texture.h>

using NodeWeakPtr = std::weak_ptr<libvrm::gltf::Node>;

template<typename T>
static void
Push(grapho::imgui::TreeSplitter* splitter, const char* label, T& values)
{
  char buf[256];
  snprintf(buf, sizeof(buf), "%s (%zd)", label, values.size());
  auto ui = splitter->Push(buf);

  for (uint32_t i = 0; i < values.size(); ++i) {
    snprintf(
      buf, sizeof(buf), "%02d:%s", i, (const char*)values[i]->Name.c_str());
    auto callback = [i, value = values[i]]() { ::ShowGui(i, value); };
    splitter->Push(buf, ui, callback);
  }
}

struct SceneGuiImpl
{
  std::map<NodeWeakPtr, std::string, std::owner_less<NodeWeakPtr>> m_map;
  std::shared_ptr<runtimescene::RuntimeScene> m_scene;
  std::shared_ptr<SceneNodeSelection> m_selection;
  float m_indent;

  grapho::imgui::TreeSplitter m_splitter;

  SceneGuiImpl(const std::shared_ptr<runtimescene::RuntimeScene>& scene,
               const std::shared_ptr<SceneNodeSelection>& selection,
               float indent)
    : m_scene(scene)
    , m_selection(selection)
    , m_indent(indent)
  {
    Push(&m_splitter, "Samplers", scene->m_table->m_samplers);
    Push(&m_splitter, "Textures", scene->m_table->m_textures);
    Push(&m_splitter, "Materials", scene->m_table->m_materials);
  }

  void Show(const char* title, bool* p_open)
  {
    int window_flags = 0;
    std::shared_ptr<libvrm::gltf::Node> showSelected;
    if (auto selected = m_selection->selected.lock()) {
      {
        if (auto mesh_index = selected->Mesh) {
          // showSelected = selected;
          window_flags |= ImGuiWindowFlags_NoScrollbar;
        }
      }
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
    if (ImGui::Begin(title, p_open, window_flags)) {
      m_splitter.ShowGui();
      // auto size = ImGui::GetContentRegionAvail();
      // m_selection->selected = m_selection->new_selected;
      //
      // if (ImGui::CollapsingHeader("textures", ImGuiTreeNodeFlags_None)) {
      // }
      // if (ImGui::CollapsingHeader("materials", ImGuiTreeNodeFlags_None)) {
      //   int i = 0;
      //   for (auto& m : m_scene->m_table->m_materials) {
      //     ShowMaterial(i++, m);
      //   }
      // }
      // if (ImGui::CollapsingHeader("meshes", ImGuiTreeNodeFlags_None)) {
      // }
      // if (ImGui::CollapsingHeader("nodes", ImGuiTreeNodeFlags_None)) {
      //   ShowNodes();
      // }
      // if (ImGui::CollapsingHeader("animations", ImGuiTreeNodeFlags_None)) {
      // }
    }
    ImGui::End();
    ImGui::PopStyleVar();
  }

  void ShowNodes()
  {
    auto size = ImGui::GetContentRegionAvail();
    m_selection->selected = m_selection->new_selected;

    std::shared_ptr<libvrm::gltf::Node> showSelected;
    if (auto selected = m_selection->selected.lock()) {
      {
        if (auto mesh_index = selected->Mesh) {
          showSelected = selected;
        }
      }
    }

    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, m_indent);
    if (showSelected) {
      if (ImGui::BeginChild("##scene-tree",
                            { size.x, size.y / 2 },
                            true,
                            ImGuiWindowFlags_None)) {

        m_scene->m_table->Traverse(
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
        if (showSelected->Mesh) {
          auto meshInstance = m_scene->GetRuntimeMesh(showSelected->Mesh);
          char morph_id[256];
          for (int i = 0; i < showSelected->Mesh->m_morphTargets.size(); ++i) {
            auto& morph = showSelected->Mesh->m_morphTargets[i];
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
      m_scene->m_table->Traverse(
        [this](const std::shared_ptr<libvrm::gltf::Node>& node) {
          return Enter(node);
        },
        [this]() { Leave(); });
    }
    ImGui::PopStyleVar();
  }

  bool Enter(const std::shared_ptr<libvrm::gltf::Node>& node)
  {
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
    if (m_selection->selected.lock() == node) {
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
      ImGui::TreeNodeEx(&*node, node_flags, "%s", Label(node).c_str());

    ImGui::PopStyleColor(push);

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
      m_selection->new_selected = node;
    }

    return node->Children.size() && node_open;
  }

  void Leave() { ImGui::TreePop(); }

  const std::string& Label(const std::shared_ptr<libvrm::gltf::Node>& node)
  {
    auto found = m_map.find(node);
    if (found != m_map.end()) {
      return found->second;
    }

    std::stringstream ss;
    if (node->Mesh) {
      ss << "󰕣 ";
      if (node->Mesh->m_morphTargets.size()) {
        ss << " ";
      }
    }

    if (auto humanoid = node->Humanoid) {
      // HumanBone = static_cast<libvrm::vrm::HumanBones>(i);
      switch (*humanoid) {
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
    for (auto& solver : m_scene->m_table->m_springBones) {
      for (auto& joint : solver->Joints) {
        auto joint_node = joint->Head;
        if (joint_node == node) {
          ss << "󰚟 ";
          break;
        }
      }
    }
    // collider
    for (auto& collider : m_scene->m_table->m_springColliders) {
      if (collider->Node == node) {
        ss << "󱥔 ";
        break;
      }
    }

    ss << node->Name;

    auto inserted = m_map.insert({ node, ss.str() });
    return inserted.first->second;
  }
};

SceneGui::SceneGui(const std::shared_ptr<runtimescene::RuntimeScene>& scene,
                   const std::shared_ptr<SceneNodeSelection>& selection,
                   float indent)
  : m_impl(new SceneGuiImpl(scene, selection, indent))
{
}

SceneGui::~SceneGui()
{
  delete m_impl;
}

void
SceneGui::Show(const char* title, bool* p_open)
{
  m_impl->Show(title, p_open);
}
