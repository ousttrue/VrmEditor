#include "tag.h"
#include <gltfjson/gltf_typing_vrm0.h>
#include <gltfjson/gltf_typing_vrm1.h>
#include <imgui.h>

static ShowTagFunc
Tags(const std::list<std::string>& tags)
{
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

// "SCALAR" "VEC2" "VEC3" "VEC4" "MAT2" "MAT3" "MAT4"
static std::string
ComponentName(gltfjson::ComponentTypes component, const std::string& type)
{
  std::unordered_map<gltfjson::ComponentTypes, std::string> map{
    { gltfjson::ComponentTypes::BYTE, "i8" },
    { gltfjson::ComponentTypes::UNSIGNED_BYTE, "u8" },
    { gltfjson::ComponentTypes::SHORT, "i16" },
    { gltfjson::ComponentTypes::UNSIGNED_SHORT, "u16" },
    { gltfjson::ComponentTypes::UNSIGNED_INT, "u32" },
    { gltfjson::ComponentTypes::FLOAT, "f32" },
  };
  std::unordered_map<std::string, std::string> type_map{
    { "SCALAR", "" }, { "VEC2", "_2" }, { "VEC3", "_3" },  { "VEC4", "_4" },
    { "MAT2", "_4" }, { "MAT3", "_9" }, { "MAT4", "_16" },
  };
  auto found = map.find(component);
  if (found != map.end()) {
    auto found2 = type_map.find(type);
    if (found2 != type_map.end()) {
      return found->second + found2->second;
    }
  }
  return "";
}

static void
Check(std::list<std::string>& tags,
      const std::string& tag,
      std::optional<uint32_t> id,
      uint32_t target)
{
  if (id && *id == target) {
    tags.push_back(tag);
  }
}

static void
FindUsage(std::list<std::string>& tags,
          const gltfjson::Root& root,
          uint32_t accessorId)
{
  for (auto mesh : root.Meshes) {
    for (auto prim : mesh.Primitives) {
      if (auto attr = prim.Attributes()) {
        Check(tags, "POS", attr->POSITION_Id(), accessorId);
        Check(tags, "NOM", attr->NORMAL_Id(), accessorId);
        Check(tags, "TEX0", attr->TEXCOORD_0_Id(), accessorId);
        Check(tags, "TEX1", attr->TEXCOORD_1_Id(), accessorId);
        Check(tags, "TEX2", attr->TEXCOORD_2_Id(), accessorId);
        Check(tags, "TEX3", attr->TEXCOORD_3_Id(), accessorId);
        Check(tags, "WEIGHT", attr->WEIGHTS_0_Id(), accessorId);
        Check(tags, "JOINT", attr->JOINTS_0_Id(), accessorId);
        Check(tags, "TANGENT", attr->TANGENT_Id(), accessorId);
        Check(tags, "COLOR", attr->COLOR_0_Id(), accessorId);
      }
      Check(tags, "indices", prim.IndicesId(), accessorId);
      for (int j = 0; j < prim.Targets.size(); ++j) {
        auto target = prim.Targets[j];
        std::stringstream ss;
        ss << "morph#" << j;
        Check(tags, ss.str() + ".POS", target.POSITION_Id(), accessorId);
        Check(tags, ss.str() + ".NOM", target.NORMAL_Id(), accessorId);
      }
    }
  }
  for (int i = 0; i < root.Animations.size(); ++i) {
    auto animation = root.Animations[i];
    for (auto sampler : animation.Samplers) {
      std::stringstream ss;
      ss << "anim#" << i;
      Check(tags, ss.str() + ".in", sampler.InputId(), accessorId);
      Check(tags, ss.str() + ".out", sampler.OutputId(), accessorId);
    }
  }
}

ShowTagFunc
AccessorTag(const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            const gltfjson::tree::NodePtr& item)
{
  if (item) {
    std::list<std::string> tags;
    for (int i = 0; i < root.Accessors.size(); ++i) {
      auto accessor = root.Accessors[i];
      if (accessor.m_json == item) {
        // scalar, vec2, vec3...
        if (auto component = accessor.ComponentType()) {
          auto type = accessor.TypeString();
          if (type.size()) {
            auto name = ComponentName((gltfjson::ComponentTypes)*component,
                                      (const char*)type.c_str());
            if (name.size()) {
              tags.push_back(name);
              // return [name]() { ImGui::SmallButton(name.c_str()); };
            }
          }
        }
        FindUsage(tags, root, i);
        break;
      }
    }
    return Tags(tags);
  }
  return {};
}

ShowTagFunc
ImageTag(const gltfjson::Root& root,
         const gltfjson::Bin& bin,
         const gltfjson::tree::NodePtr& item)
{
  if (item) {
    auto image = gltfjson::Image(item);
    auto mime = image.MimeTypeString();
    if (mime.size()) {
      return [mime]() { ImGui::SmallButton((const char*)mime.c_str()); };
    }
    auto uri = image.UriString();
    if (uri.size()) {
      std::filesystem::path path(uri);
      return [ext = path.extension().string()]() {
        ImGui::SmallButton(ext.c_str());
      };
    }
  }

  return {};
}

ShowTagFunc
MaterialTag(const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            const gltfjson::tree::NodePtr& item)
{
  if (item) {
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
  }

  return {};
}

ShowTagFunc
NodeTag(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        const gltfjson::tree::NodePtr& item)
{
  std::list<std::string> tags;
  if (item) {
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
  }

  return Tags(tags);
}
