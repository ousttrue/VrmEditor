#include "importer.h"
#include "gltfroot.h"
#include "node.h"
#include <DirectXMath.h>
#include <gltfjson.h>
#include <gltfjson/gltf_typing_vrm0.h>
#include <gltfjson/gltf_typing_vrm1.h>
#include <gltfjson/json_tree_parser.h>

namespace libvrm {

static DirectX::XMFLOAT4
RotateY180(const DirectX::XMFLOAT4& src)
{
  auto r = DirectX::XMLoadFloat4(&src);
  auto y180 = DirectX::XMQuaternionRotationMatrix(
    DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(180.0f)));
  DirectX::XMFLOAT4 dst;
  DirectX::XMStoreFloat4(&dst, DirectX::XMQuaternionMultiply(y180, r));
  return dst;
}

static std::shared_ptr<Node>
ParseNode(const std::shared_ptr<GltfRoot>& scene,
          int i,
          const gltfjson::Node& node)
{
  auto ptr = std::make_shared<Node>(node.NameString());

  if (node.Matrix.size() == 16) {
    // matrix
    auto& m = node.Matrix;
    auto local = DirectX::XMFLOAT4X4{
      m[0],  m[1],  m[2],  m[3],  //
      m[4],  m[5],  m[6],  m[7],  //
      m[8],  m[9],  m[10], m[11], //
      m[12], m[13], m[14], m[15], //
    };
    ptr->SetLocalInitialMatrix(DirectX::XMLoadFloat4x4(&local));
  } else {
    // T
    if (node.Translation.size() == 3) {
      auto& t = node.Translation;
      ptr->InitialTransform.Translation = { t[0], t[1], t[2] };
    }
    // R
    if (node.Rotation.size() == 4) {
      auto& r = node.Rotation;
      ptr->InitialTransform.Rotation = { r[0], r[1], r[2], r[3] };
    }
    // S
    if (node.Scale.size() == 3) {
      auto& s = node.Scale;
      ptr->InitialScale = { s[0], s[1], s[2] };
    }
  }

  return ptr;
}

static bool
Parse(const std::shared_ptr<GltfRoot>& scene)
{
  scene->m_title = "glTF";

  if (auto required = scene->m_gltf->ExtensionsRequired()) {
    if (auto array =
          std::dynamic_pointer_cast<gltfjson::tree::ArrayNode>(required)) {
      for (auto ex : array->Value) {
        auto name = ex->U8String();
        if (name == u8"KHR_draco_mesh_compression") {
          // return std::unexpected{ "KHR_draco_mesh_compression" };
          return {};
        }
        if (name == u8"KHR_mesh_quantization") {
          // return std::unexpected{ "KHR_mesh_quantization" };
          return {};
        }
      }
    }
  }

  {
    auto& nodes = scene->m_gltf->Nodes;
    for (int i = 0; i < nodes.size(); ++i) {
      if (auto node = ParseNode(scene, i, nodes[i])) {
        scene->m_nodes.push_back(node);
      } else {
        // return std::unexpected{ node.error() };
        return {};
      }
    }
    for (int i = 0; i < nodes.size(); ++i) {
      auto node = nodes[i];
      for (auto child : node.Children) {
        Node::AddChild(scene->m_nodes[i], scene->m_nodes[child]);
      }
    }
  }

  if (scene->m_gltf->Scenes.size()) {
    auto _scene = scene->m_gltf->Scenes[0];
    for (auto node : _scene.Nodes) {
      scene->m_roots.push_back(scene->m_nodes[node]);
    }
  }

  // humanoid
  if (auto VRMC_vrm_animation =
        scene->m_gltf->GetExtension<gltfjson::vrm1::VRMC_vrm_animation>()) {
    scene->m_type = ModelType::VrmA;
    scene->m_title = "vrm-animation";
    if (auto humanoid = VRMC_vrm_animation->Humanoid()) {
      if (auto humanBones = humanoid->HumanBones()) {
        if (auto object = std::dynamic_pointer_cast<gltfjson::tree::ObjectNode>(
              humanBones->m_json)) {
          for (auto& kv : object->Value) {
            auto name = kv.first;
            if (auto bone = HumanBoneFromName(gltfjson::from_u8(name),
                                              VrmVersion::_1_0)) {
              auto index = (uint32_t)*kv.second->Get(u8"node")->Ptr<float>();
              scene->m_nodes[index]->Humanoid = *bone;
            } else {
              std::cout << gltfjson::from_u8(name) << std::endl;
            }
          }
        }
      }
    }
    if (auto pose =
          VRMC_vrm_animation->GetExtension<gltfjson::vrm1::VRMC_vrm_pose>()) {
      if (auto humanoid = pose->Humanoid()) {
      }
    }
  } else if (auto VRMC_vrm =
               scene->m_gltf->GetExtension<gltfjson::vrm1::VRMC_vrm>()) {
    scene->m_type = ModelType::Vrm1;
    scene->m_title = "vrm-1.0";
    if (auto humanoid = VRMC_vrm->Humanoid()) {
      if (auto humanBones = humanoid->HumanBones()) {
        if (auto object = std::dynamic_pointer_cast<gltfjson::tree::ObjectNode>(
              humanBones->m_json)) {
          for (auto& kv : object->Value) {
            auto name = kv.first;
            if (auto bone = HumanBoneFromName(gltfjson::from_u8(name),
                                              VrmVersion::_1_0)) {
              auto index = (uint32_t)*kv.second->Get(u8"node")->Ptr<float>();
              scene->m_nodes[index]->Humanoid = *bone;
            } else {
              std::cout << gltfjson::from_u8(name) << std::endl;
            }
          }
        }
      }
    }
  } else if (auto VRM = scene->m_gltf->GetExtension<gltfjson::vrm0::VRM>()) {
    scene->m_type = ModelType::Vrm0;
    scene->m_title = "vrm-0.x";
    if (auto humanoid = VRM->Humanoid()) {
      // bone & node
      for (auto humanBone : humanoid->HumanBones) {
        if (auto node = humanBone.NodeId()) {
          auto index = *node;
          auto name = humanBone.BoneString();
          // std::cout << name << ": " << index << std::endl;
          if (auto bone =
                HumanBoneFromName(gltfjson::from_u8(name), VrmVersion::_0_x)) {
            scene->m_nodes[index]->Humanoid = *bone;
          }
        }
      }
    }

    // ROTATE Y180 VRM-0.X TPose
    for (auto& root : scene->m_roots) {
      root->InitialTransform.Rotation =
        RotateY180(root->InitialTransform.Rotation);
    }
  }

  scene->InitializeNodes();

  return true;
}

static bool
Load(const std::shared_ptr<GltfRoot>& scene,
     std::span<const uint8_t> json_chunk,
     std::span<const uint8_t> bin_chunk,
     const std::shared_ptr<gltfjson::Directory>& dir)
{
  gltfjson::tree::Parser parser(json_chunk);
  if (auto result = parser.Parse()) {
    scene->m_gltf = std::make_shared<gltfjson::Root>(result);
    scene->m_bin = { dir, bin_chunk };
    if (!scene->m_bin.Dir) {
      scene->m_bin.Dir = std::make_shared<gltfjson::Directory>();
    }
    return Parse(scene);
  } else {
    // auto error = result.error();
    // std::string msg{ (const char*)error.data(),
    //                  (const char*)error.data() + error.size() };
    // return std::unexpected{ msg };
    return {};
  }
}

bool
LoadBytes(const std::shared_ptr<GltfRoot>& scene,
          std::span<const uint8_t> bytes,
          const std::shared_ptr<gltfjson::Directory>& dir)
{
  scene->m_bytes.assign(bytes.begin(), bytes.end());
  if (auto glb = gltfjson::Glb::Parse(scene->m_bytes)) {
    // as glb
    return Load(scene, glb->JsonChunk, glb->BinChunk, dir);
  }

  // try gltf
  return Load(scene, scene->m_bytes, {}, dir);
}

std::shared_ptr<GltfRoot>
LoadPath(const std::filesystem::path& path)
{
  if (auto bytes = gltfjson::ReadAllBytes(path)) {
    auto ptr = std::make_shared<GltfRoot>();
    if (auto load = LoadBytes(
          ptr,
          *bytes,
          std::make_shared<gltfjson::Directory>(path.parent_path()))) {
      return ptr;
    } else {
      // return std::unexpected(load.error());
      return {};
    }
  } else {
    // return std::unexpected{ bytes.error() };
    return {};
  }
}

std::shared_ptr<GltfRoot>
LoadGltf(const std::string& json)
{
  auto ptr = std::make_shared<GltfRoot>();
  std::span<const uint8_t> bytes{ (const uint8_t*)json.data(), json.size() };
  if (auto load = LoadBytes(ptr, bytes, nullptr)) {
    return ptr;
  } else {
    // return std::unexpected(load.error());
    return {};
  }
}

} // namespace
