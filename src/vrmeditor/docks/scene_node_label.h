#pragma once
#include <string>
#include <vrm/humanbones.h>
#include <vrm/mesh.h>
#include <vrm/node.h>
#include <vrm/scene.h>
#include <vrm/springbone.h>

using NodeWeakPtr = std::weak_ptr<libvrm::gltf::Node>;

class SceneGui
{
  std::map<NodeWeakPtr, std::string, std::owner_less<NodeWeakPtr>> m_map;

public:
  const std::string& Label(const libvrm::gltf::Scene& scene,
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
    if (scene.m_colliderGroups.size()) {
    }

    ss << node->Name;

    auto inserted = m_map.insert({ node, ss.str() });
    return inserted.first->second;
  }
};
