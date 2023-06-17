#include "tag.h"
#include <gltfjson/gltf_typing_vrm0.h>
#include <gltfjson/gltf_typing_vrm1.h>
#include <imgui.h>

ShowTagFunc
ImageTag(const gltfjson::Root& root,
         const gltfjson::Bin& bin,
         const gltfjson::tree::NodePtr& item)
{
  auto image = gltfjson::Image(item);

  return {};
}

ShowTagFunc
MaterialTag(const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            const gltfjson::tree::NodePtr& item)
{

  for (int i = 0; i < root.Materials.size(); ++i) {
    auto m = root.Materials[i];
    if (m.m_json == item) {
      if (m.GetExtension<gltfjson::vrm1::MToon>()) {
        return []() { ImGui::SmallButton("mtoon1"); };
      } else if (gltfjson::vrm0::GetVrmMaterial(root, i)) {
        return []() { ImGui::SmallButton("mtoon0"); };
      } else if (m.GetExtension<gltfjson::KHR_materials_unlit>()) {
        return []() { ImGui::SmallButton("unlit"); };
      } else {
        return []() { ImGui::SmallButton("pbr"); };
      }
    }
  }

  return {};
}

ShowTagFunc
NodeTag(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        const gltfjson::tree::NodePtr& item)
{
  std::list<std::string> tags;
  auto vrma = root.GetExtension<gltfjson::vrm1::VRMC_vrm_animation>();
  auto vrm1 = root.GetExtension<gltfjson::vrm1::VRMC_vrm>();
  auto vrm0 = root.GetExtension<gltfjson::vrm0::VRM>();
  for (int i = 0; i < root.Nodes.size(); ++i) {
    auto node = root.Nodes[i];
    if (node.m_json == item) {
      if (node.MeshId()) {
        tags.push_back("mesh");
      }
      if (node.SkinId()) {
        tags.push_back("skin");
      }
      if (node.CameraId()) {
        tags.push_back("camera");
      }
      if (auto extensions = node.Extensions()) {
        if (extensions->Get(u8"KHR_lights_punctual")) {
          tags.push_back("light");
        }
      }
      if (vrma) {
        if (auto humanoid = vrma->Humanoid()) {
          if (auto humanBones = humanoid->HumanBones()) {
            if (auto obj = humanBones->m_json->Object()) {
              for (auto kv : *obj) {
                if (auto node = kv.second->Get(u8"node")) {
                  if (auto p = node->Ptr<float>()) {
                    if (*p == i) {
                      tags.push_back(
                        { (const char*)kv.first.data(), kv.first.size() });
                    }
                  }
                }
              }
            }
          }
        }
      }
      if (vrm1) {
        if (auto humanoid = vrm1->Humanoid()) {
          if (auto humanBones = humanoid->HumanBones()) {
            if (auto obj = humanBones->m_json->Object()) {
              for (auto kv : *obj) {
                if (auto node = kv.second->Get(u8"node")) {
                  if (auto p = node->Ptr<float>()) {
                    if (*p == i) {
                      tags.push_back(
                        { (const char*)kv.first.data(), kv.first.size() });
                    };
                  }
                }
              }
            }
          }
        }
      }
      if (vrm0) {
        if (auto humanoid = vrm0->Humanoid()) {
          for (auto humanBone : humanoid->HumanBones) {
            if (auto node = humanBone.NodeId()) {
              if (*node == i) {
                auto bone = humanBone.BoneString();
                tags.push_back({ (const char*)bone.data(), bone.size() });
              }
            }
          }
        }
      }
      break;
    }
  }

  return [tags]() {
    auto first = true;
    for (auto& tag : tags) {
      if (first) {
        first = false;
      } else {
        ImGui::SameLine();
      }
      ImGui::SmallButton(tag.c_str());
    }
  };
}
