#include "vrm/scene.h"
#include "vrm/animation.h"
#include "vrm/bvh.h"
#include "vrm/dmath.h"
#include "vrm/glb.h"
#include "vrm/jsonpath.h"
#include "vrm/material.h"
#include "vrm/mesh.h"
#include "vrm/node.h"
#include "vrm/skin.h"
#include "vrm/springbone.h"
#include <DirectXMath.h>
#include <array>
#include <expected>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <vector>

// matrix
inline void
to_json(nlohmann::json& j, const DirectX::XMFLOAT4X4& m)
{
  j = nlohmann::json{
    m._11, m._12, m._13, m._14, m._21, m._22, m._23, m._24,
    m._31, m._32, m._33, m._34, m._41, m._42, m._43, m._44,
  };
}
inline void
from_json(const nlohmann::json& j, DirectX::XMFLOAT4X4& m)
{
  m._11 = j[0];
  m._12 = j[1];
  m._13 = j[2];
  m._14 = j[3];
  m._21 = j[4];
  m._22 = j[5];
  m._23 = j[6];
  m._24 = j[7];
  m._31 = j[8];
  m._32 = j[9];
  m._33 = j[10];
  m._34 = j[11];
  m._41 = j[12];
  m._42 = j[13];
  m._43 = j[14];
  m._44 = j[15];
}

namespace libvrm::gltf {
Scene::Scene() {}

Scene::~Scene()
{
  std::cout << "Scene::~Scene()" << std::endl;
}

std::expected<std::shared_ptr<Scene>, std::string>
Scene::LoadPath(const std::filesystem::path& path)
{
  if (auto bytes = ReadAllBytes(path)) {
    auto ptr = std::make_shared<Scene>();
    if (auto load = ptr->LoadBytes(
          *bytes, std::make_shared<Directory>(path.parent_path()))) {
      return ptr;
    } else {
      return std::unexpected(load.error());
    }
  } else {
    return std::unexpected{ bytes.error() };
  }
}

std::expected<bool, std::string>
Scene::LoadBytes(std::span<const uint8_t> bytes,
                 const std::shared_ptr<Directory>& dir)
{
  m_bytes.assign(bytes.begin(), bytes.end());
  if (auto glb = Glb::Parse(m_bytes)) {
    // as glb
    return Load(glb->JsonChunk, glb->BinChunk, dir);
  }

  // try gltf
  return Load(m_bytes, {}, dir);
}

std::expected<bool, std::string>
Scene::Load(std::span<const uint8_t> json_chunk,
            std::span<const uint8_t> bin_chunk,
            const std::shared_ptr<Directory>& dir)
{
  try {
    auto parsed = nlohmann::json::parse(json_chunk);
    m_gltf = { dir, parsed, bin_chunk };
    if (!m_gltf.Dir) {
      m_gltf.Dir = std::make_shared<Directory>();
    }
    return Parse();
  } catch (nlohmann::json::parse_error& e) {
    return std::unexpected{ e.what() };
  } catch (nlohmann::json::type_error& e) {
    return std::unexpected{ e.what() };
  } catch (std::runtime_error& e) {
    return std::unexpected{ e.what() };
  }
}

std::expected<bool, std::string>
Scene::Parse()
{
  m_title = "glTF";

  if (has(m_gltf.Json, "extensionsRequired")) {
    for (auto& ex : m_gltf.Json.at("extensionsRequired")) {
      if (ex == "KHR_draco_mesh_compression") {
        return std::unexpected{ "KHR_draco_mesh_compression" };
      }
      if (ex == "KHR_mesh_quantization") {
        return std::unexpected{ "KHR_mesh_quantization" };
      }
    }
  }

  if (has(m_gltf.Json, "extensions")) {
    auto& extensions = m_gltf.Json.at("extensions");
    if (has(extensions, "VRM")) {
      auto VRM = extensions.at("VRM");
      // TODO: meta
      m_type = ModelType::Vrm0;
      m_title = "vrm-0.x";
    }
    if (has(extensions, "VRMC_vrm")) {
      m_type = ModelType::Vrm1;
      m_title = "vrm-1.0";
    }
  }

  if (has(m_gltf.Json, "samplers")) {
    auto& samplers = m_gltf.Json.at("samplers");
    for (int i = 0; i < samplers.size(); ++i) {
      if (auto sampler = ParseTextureSampler(i, samplers.at(i))) {
        m_samplers.push_back(*sampler);
      } else {
        return std::unexpected{ sampler.error() };
      }
    }
  }

  if (has(m_gltf.Json, "images")) {
    auto& images = m_gltf.Json.at("images");
    for (int i = 0; i < images.size(); ++i) {
      if (auto image = ParseImage(i, images.at(i))) {
        m_images.push_back(*image);
      } else {
        return std::unexpected{ image.error() };
      }
    }
  }

  if (has(m_gltf.Json, "textures")) {
    auto& textures = m_gltf.Json.at("textures");
    for (int i = 0; i < textures.size(); ++i) {
      if (auto texture = ParseTexture(i, textures.at(i))) {
        m_textures.push_back(*texture);
      } else {
        return std::unexpected{ texture.error() };
      }
    }
  }

  if (has(m_gltf.Json, "materials")) {
    auto& materials = m_gltf.Json.at("materials");
    for (int i = 0; i < materials.size(); ++i) {
      if (auto material = ParseMaterial(i, materials.at(i))) {
        m_materials.push_back(*material);
      } else {
        return std::unexpected{ material.error() };
      }
    }
  }

  if (has(m_gltf.Json, "meshes")) {
    auto& meshes = m_gltf.Json.at("meshes");
    for (int i = 0; i < meshes.size(); ++i) {
      if (auto mesh = ParseMesh(i, meshes.at(i))) {
        m_meshes.push_back(*mesh);
      } else {
        return std::unexpected{ mesh.error() };
      }
    }
  }

  if (has(m_gltf.Json, "skins")) {
    auto skins = m_gltf.Json.at("skins");
    for (int i = 0; i < skins.size(); ++i) {
      if (auto skin = ParseSkin(i, skins.at(i))) {
        m_skins.push_back(*skin);
      } else {
        return std::unexpected{ skin.error() };
      }
    }
  }

  if (has(m_gltf.Json, "nodes")) {
    auto nodes = m_gltf.Json.at("nodes");
    for (int i = 0; i < nodes.size(); ++i) {
      if (auto node = ParseNode(i, nodes.at(i))) {
        m_nodes.push_back(*node);
      } else {
        return std::unexpected{ node.error() };
      }
    }
    for (int i = 0; i < nodes.size(); ++i) {
      auto& node = nodes.at(i);
      if (has(node, "children")) {
        for (auto child : node.at("children")) {
          gltf::Node::AddChild(m_nodes[i], m_nodes[child]);
        }
      }
    }
  }
  if (has(m_gltf.Json, "scenes")) {
    auto scene = m_gltf.Json.at("scenes").at(0);
    for (auto& node : scene.at("nodes")) {
      m_roots.push_back(m_nodes[node]);
    }
  }

  // calc world
  auto enter = [](const std::shared_ptr<gltf::Node>& node) {
    node->CalcWorldMatrix();
    node->CalcInitialMatrix();
    return true;
  };
  Traverse(enter, {});

  if (has(m_gltf.Json, "animations")) {
    auto& animations = m_gltf.Json.at("animations");
    for (int i = 0; i < animations.size(); ++i) {
      if (auto animation = ParseAnimation(i, animations.at(i))) {
        m_animations.push_back(*animation);
      } else {
        return std::unexpected{ animation.error() };
      }
    }
  }

  if (m_type == ModelType::Vrm0) {
    if (auto vrm0 = ParseVrm0()) {
    } else {
      return std::unexpected{ vrm0.error() };
    }
  }

  if (m_type == ModelType::Vrm1) {
    if (auto vrm1 = ParseVrm1()) {
    } else {
      return std::unexpected{ vrm1.error() };
    }
  }

  InitializeNodes();

  return true;
}

std::expected<std::shared_ptr<TextureSampler>, std::string>
Scene::ParseTextureSampler(int i, const nlohmann::json& sampler)
{
  auto ptr = std::make_shared<gltf::TextureSampler>();
  return ptr;
}

std::expected<std::shared_ptr<gltf::Image>, std::string>
Scene::ParseImage(int i, const nlohmann::json& image)
{
  std::span<const uint8_t> bytes;
  if (has(image, "bufferView")) {
    if (auto buffer_view = m_gltf.buffer_view(image.at("bufferView"))) {
      bytes = *buffer_view;
    } else {
      return std::unexpected{ buffer_view.error() };
    }
  } else if (has(image, "uri")) {
    if (auto buffer_view = m_gltf.Dir->GetBuffer(image.at("uri"))) {
      bytes = *buffer_view;
    } else {
      return std::unexpected{ buffer_view.error() };
    }
  } else {
    return std::unexpected{ "not bufferView nor uri" };
  }
  std::stringstream ss;
  ss << "image" << i;
  auto name = image.value("name", ss.str());
  auto ptr = std::make_shared<gltf::Image>(name);
  if (!ptr->Load(bytes)) {
    return std::unexpected{ name };
  }
  return ptr;
}

std::expected<std::shared_ptr<gltf::Texture>, std::string>
Scene::ParseTexture(int i, const nlohmann::json& texture)
{
  std::stringstream ss;
  ss << "image" << i;
  auto name = texture.value("name", ss.str());
  auto ptr = std::make_shared<gltf::Texture>();
  ptr->Source = m_images[texture.at("source")];
  if (has(texture, "sampler")) {
    ptr->Sampler = m_samplers[texture.at("sampler")];
  }
  return ptr;
}

std::expected<std::shared_ptr<gltf::Material>, std::string>
Scene::ParseMaterial(int i, const nlohmann::json& material)
{
  std::stringstream ss;
  ss << "material" << i;
  auto ptr = std::make_shared<gltf::Material>(material.value("name", ss.str()));

  if (has(material, "pbrMetallicRoughness")) {
    auto pbrMetallicRoughness = material.at("pbrMetallicRoughness");
    if (has(pbrMetallicRoughness, "baseColorTexture")) {
      auto& baseColorTexture = pbrMetallicRoughness.at("baseColorTexture");
      int texture_index = baseColorTexture.at("index");
      ptr->ColorTexture = m_textures[texture_index];
      // auto& textures = m_gltf.Json.at("textures");
      // auto texture = textures.at(texture_index);
      // int image_index = texture.at("source");
      // ptr->Texture.Image = m_images[image_index];
    }
  }
  return ptr;
}

std::expected<std::shared_ptr<gltf::Mesh>, std::string>
Scene::ParseMesh(int i, const nlohmann::json& mesh)
{
  auto ptr = std::make_shared<gltf::Mesh>();
  if (has(mesh, "name")) {
    ptr->Name = mesh.at("name");
  }
  const nlohmann::json* lastAtributes = nullptr;
  for (auto& prim : mesh.at("primitives")) {
    std::shared_ptr<gltf::Material> material;
    if (has(prim, "material")) {
      material = m_materials[prim.at("material")];
    } else {
      // default material
      material = std::make_shared<gltf::Material>("default");
    }

    const nlohmann::json& attributes = prim.at("attributes");
    if (lastAtributes && attributes == *lastAtributes) {
      // for vrm shared vertex buffer
      if (auto expected = AddIndices(0, ptr.get(), prim, material)) {
        // OK
      } else {
        return std::unexpected{ expected.error() };
      }
    } else {
      // extend vertex buffer
      std::span<const DirectX::XMFLOAT3> positions;
      if (auto accessor = m_gltf.accessor<DirectX::XMFLOAT3>(
            attributes[gltf::VERTEX_POSITION])) {
        positions = *accessor;
      } else {
        return std::unexpected{ accessor.error() };
      }
      std::vector<DirectX::XMFLOAT3> copy;
      if (m_type == ModelType::Vrm0) {
        copy.reserve(positions.size());
        for (auto& p : positions) {
          copy.push_back({ -p.x, p.y, -p.z });
        }
        positions = copy;
      }
      auto offset = ptr->addPosition(positions);

      if (has(attributes, gltf::VERTEX_NORMAL)) {
        if (auto accessor = m_gltf.accessor<DirectX::XMFLOAT3>(
              attributes.at(gltf::VERTEX_NORMAL))) {
          ptr->setNormal(offset, *accessor);
        } else {
          return std::unexpected{ accessor.error() };
        }
      }

      if (has(attributes, gltf::VERTEX_UV)) {
        if (auto accessor = m_gltf.accessor<DirectX::XMFLOAT2>(
              attributes.at(gltf::VERTEX_UV))) {
          ptr->setUv(offset, *accessor);
        } else {
          return std::unexpected{ accessor.error() };
        }
      }

      if (has(attributes, gltf::VERTEX_JOINT) &&
          has(attributes, gltf::VERTEX_WEIGHT)) {
        // skinning
        int joint_accessor = attributes.at(gltf::VERTEX_JOINT);
        switch (
          *gltf::item_size(m_gltf.Json.at("accessors").at(joint_accessor))) {
          case 8:
            if (auto accessor = m_gltf.accessor<ushort4>(joint_accessor)) {
              if (auto accessor_w = m_gltf.accessor<DirectX::XMFLOAT4>(
                    attributes.at(gltf::VERTEX_WEIGHT))) {
                ptr->setBoneSkinning(offset, *accessor, *accessor_w);
              } else {
                return std::unexpected{ accessor_w.error() };
              }
            } else {
              return std::unexpected{ accessor.error() };
            }
            break;

          default:
            // not implemented
            return std::unexpected{ "JOINTS_0 is not ushort4" };
        }
      }

      // extend morph target
      if (has(prim, "targets")) {
        auto& targets = prim.at("targets");
        for (int i = 0; i < targets.size(); ++i) {
          auto& target = targets.at(i);
          auto morph = ptr->getOrCreateMorphTarget(i);
          // std::cout << target << std::endl;
          std::span<const DirectX::XMFLOAT3> positions;
          if (auto accessor = m_gltf.accessor<DirectX::XMFLOAT3>(
                target.at(gltf::VERTEX_POSITION))) {
            positions = *accessor;
          } else {
            return std::unexpected{ accessor.error() };
          }
          std::vector<DirectX::XMFLOAT3> copy;
          if (m_type == ModelType::Vrm0) {
            copy.reserve(positions.size());
            for (auto& p : positions) {
              copy.push_back({ -p.x, p.y, -p.z });
            }
            positions = copy;
          }
          /*auto morphOffset =*/morph->addPosition(positions);
        }
      }

      // extend indices and add vertex offset
      if (auto expected = AddIndices(offset, ptr.get(), prim, material)) {
        // OK
      } else {
        return std::unexpected{ expected.error() };
      }
    }

    // find morph target name
    // primitive.extras.targetNames
    if (has(prim, "extras")) {
      auto& extras = prim.at("extras");
      if (has(extras, "targetNames")) {
        auto& names = extras.at("targetNames");
        // std::cout << names << std::endl;
        for (int i = 0; i < names.size(); ++i) {
          ptr->getOrCreateMorphTarget(i)->Name = names[i];
        }
      }
    }

    lastAtributes = &attributes;
  }

  // find morph target name
  // mesh.extras.targetNames
  if (has(mesh, "extras")) {
    auto& extras = mesh.at("extras");
    if (has(extras, "targetNames")) {
      auto& names = extras.at("targetNames");
      // std::cout << names << std::endl;
      for (int i = 0; i < names.size(); ++i) {
        ptr->getOrCreateMorphTarget(i)->Name = names[i];
      }
    }
  }

  return ptr;
}

std::expected<std::shared_ptr<gltf::Skin>, std::string>
Scene::ParseSkin(int i, const nlohmann::json& skin)
{
  auto ptr = std::make_shared<gltf::Skin>();

  std::stringstream ss;
  ss << "skin" << i;
  ptr->Name = skin.value("name", ss.str());

  for (auto& joint : skin.at("joints")) {
    ptr->Joints.push_back(joint);
  }

  std::span<const DirectX::XMFLOAT4X4> matrices;
  if (auto accessor =
        m_gltf.accessor<DirectX::XMFLOAT4X4>(skin.at("inverseBindMatrices"))) {
    matrices = *accessor;
  } else {
    return std::unexpected{ accessor.error() };
  }
  std::vector<DirectX::XMFLOAT4X4> copy;
  if (m_type == ModelType::Vrm0) {
    copy.reserve(matrices.size());
    for (auto& m : matrices) {
      copy.push_back(m);
      copy.back()._41 = -m._41;
      copy.back()._43 = -m._43;
    }
    matrices = copy;
  }
  ptr->BindMatrices.assign(matrices.begin(), matrices.end());

  assert(ptr->Joints.size() == ptr->BindMatrices.size());

  if (has(skin, "skeleton")) {
    ptr->Root = skin.at("skeleton");
  }
  return ptr;
}

std::expected<std::shared_ptr<gltf::Node>, std::string>
Scene::ParseNode(int i, const nlohmann::json& node)
{
  std::stringstream ss;
  ss << "node" << i;
  auto name = node.value("name", ss.str());
  auto ptr = std::make_shared<gltf::Node>(name);

  if (has(node, "matrix")) {
    // matrix
    auto m = node.at("matrix");
    auto local = DirectX::XMFLOAT4X4{
      m[0],  m[1],  m[2],  m[3],  //
      m[4],  m[5],  m[6],  m[7],  //
      m[8],  m[9],  m[10], m[11], //
      m[12], m[13], m[14], m[15], //
    };
    ptr->SetLocalMatrix(DirectX::XMLoadFloat4x4(&local));
  } else {
    // T
    ptr->Transform.Translation =
      node.value("translation", DirectX::XMFLOAT3{ 0, 0, 0 });
    if (m_type == ModelType::Vrm0) {
      // rotate: Y180
      auto t = ptr->Transform.Translation;
      ptr->Transform.Translation = { -t.x, t.y, -t.z };
    }
    // R
    ptr->Transform.Rotation =
      node.value("rotation", DirectX::XMFLOAT4{ 0, 0, 0, 1 });
    // S
    ptr->Scale = node.value("scale", DirectX::XMFLOAT3{ 1, 1, 1 });
  }
  if (has(node, "mesh")) {
    auto mesh_index = node.at("mesh");
    ptr->Mesh = mesh_index;

    if (has(node, "skin")) {
      int skin_index = node.at("skin");
      auto skin = m_skins[skin_index];
      ptr->Skin = skin;
    }

    ptr->Instance = std::make_shared<gltf::MeshInstance>(m_meshes[mesh_index]);
  }
  return ptr;
}

std::expected<std::shared_ptr<gltf::Animation>, std::string>
Scene::ParseAnimation(int i, const nlohmann::json& animation)
{
  std::stringstream ss;
  ss << "animation" << i;
  auto ptr =
    std::make_shared<gltf::Animation>(animation.value("name", ss.str()));

  // samplers
  auto& samplers = animation.at("samplers");

  // channels
  auto& channels = animation.at("channels");
  for (auto& channel : channels) {
    int sampler_index = channel.at("sampler");
    auto sampler = samplers[sampler_index];

    auto target = channel.at("target");
    int node_index = target.at("node");
    std::string path = target.at("path");

    // time
    int input_index = sampler.at("input");
    if (auto times = m_gltf.accessor<float>(input_index)) {
      int output_index = sampler.at("output");
      if (path == "translation") {
        if (auto values = m_gltf.accessor<DirectX::XMFLOAT3>(output_index)) {
          ptr->AddTranslation(node_index,
                              *times,
                              *values,
                              m_nodes[node_index]->Name + "-translation");
        } else {
          return std::unexpected{ values.error() };
        }
      } else if (path == "rotation") {
        if (auto values = m_gltf.accessor<DirectX::XMFLOAT4>(output_index)) {
          ptr->AddRotation(node_index,
                           *times,
                           *values,
                           m_nodes[node_index]->Name + "-rotation");
        } else {
          return std::unexpected{ values.error() };
        }
      } else if (path == "scale") {
        if (auto values = m_gltf.accessor<DirectX::XMFLOAT3>(output_index)) {
          ptr->AddScale(
            node_index, *times, *values, m_nodes[node_index]->Name + "-scale");
        } else {
          return std::unexpected{ values.error() };
        }
      } else if (path == "weights") {
        if (auto values = m_gltf.accessor<float>(output_index)) {
          auto node = m_nodes[node_index];
          if (auto mesh_index = node->Mesh) {
            auto mesh = m_meshes[*mesh_index];
            if (values->size() != mesh->m_morphTargets.size() * times->size()) {
              return std::unexpected{ "animation-weights: size not match" };
            }
            ptr->AddWeights(
              node_index, *times, *values, node->Name + "-weights");
          } else {
            return std::unexpected{ "animation-weights: no node.mesh" };
          }
        } else {
          return std::unexpected{ values.error() };
        }
      } else {
        return std::unexpected{ "animation path is not implemented: " + path };
      }
    } else {
      return std::unexpected{ times.error() };
    }
  }

  return ptr;
}

std::expected<bool, std::string>
Scene::ParseVrm0()
{
  if (!has(m_gltf.Json, "extensions")) {
    return std::unexpected{ "no extensions" };
  }
  auto& extensions = m_gltf.Json.at("extensions");

  if (!has(extensions, "VRM")) {
    return std::unexpected{ "no extensions.VRM" };
  }
  auto VRM = extensions.at("VRM");

  if (has(VRM, "humanoid")) {
    auto& humanoid = VRM.at("humanoid");
    if (has(humanoid, "humanBones")) {
      auto& humanBones = humanoid.at("humanBones");
      // bone & node
      for (auto& humanBone : humanBones) {
        int index = humanBone.at("node");
        std::string_view name = humanBone.at("bone");
        // std::cout << name << ": " << index << std::endl;
        if (auto bone = vrm::HumanBoneFromName(name, vrm::VrmVersion::_0_x)) {
          m_nodes[index]->Humanoid = NodeHumanoidInfo{
            .HumanBone = *bone,
          };
        }
      }
    }
  }

  // meta
  // specVersion
  // exporterVersion

  // firstPerson
  // materialProperties
  if (has(VRM, "materialProperties")) {
    auto& props = VRM.at("materialProperties");
    for (int i = 0; i < props.size(); ++i) {
      if (props[i].at("shader") == "VRM/MToon") {
        // TODO
        m_materials[i]->Type = MaterialTypes::MToon0;
      }
    }
  }

  if (has(VRM, "blendShapeMaster")) {

    m_expressions = std::make_shared<vrm::Expressions>();

    auto& blendShapeMaster = VRM.at("blendShapeMaster");
    if (has(blendShapeMaster, "blendShapeGroups")) {
      auto& blendShapeGroups = blendShapeMaster.at("blendShapeGroups");
      for (auto& g : blendShapeGroups) {
        // {"binds":[],"isBinary":false,"materialValues":[],"name":"Neutral","presetName":"neutral"}
        // std::cout << g << std::endl;
        auto expression = m_expressions->addBlendShape(
          g.at("presetName"), g.at("name"), g.value("isBinary", false));
        if (has(g, "binds")) {
          for (vrm::ExpressionMorphTargetBind bind : g.at("binds")) {
            // [0-100] to [0-1]
            bind.weight *= 0.01f;
            for (auto& node : m_nodes) {
              if (node->Mesh == bind.mesh) {
                bind.Node = node;
                break;
              }
            }
            expression->morphBinds.push_back(bind);
          }
        }
      }
    }
  }

  if (has(VRM, "secondaryAnimation")) {
    auto& secondaryAnimation = VRM.at("secondaryAnimation");
    if (has(secondaryAnimation, "colliderGroups")) {
      auto& colliderGroups = secondaryAnimation.at("colliderGroups");
      for (auto& colliderGroup : colliderGroups) {
        auto group = std::make_shared<vrm::ColliderGroup>();
        // *group = colliderGroup;
        m_colliderGroups.push_back(group);
      }
    }
    if (has(secondaryAnimation, "boneGroups")) {
      auto& boneGroups = secondaryAnimation.at("boneGroups");
      for (auto& boneGroup : boneGroups) {
        float stiffness = boneGroup.at("stiffiness");
        float dragForce = boneGroup.at("dragForce");
        for (auto& bone : boneGroup.at("bones")) {
          auto spring = std::make_shared<vrm::SpringSolver>();
          spring->AddRecursive(m_nodes[bone], dragForce, stiffness);
          m_springSolvers.push_back(spring);
        }
      }
    }
  }

  return true;
}

std::expected<bool, std::string>
Scene::ParseVrm1()
{
  if (!has(m_gltf.Json, "extensions")) {
    return std::unexpected{ "no extensions" };
  }
  auto& extensions = m_gltf.Json.at("extensions");

  if (!has(extensions, "VRMC_vrm")) {
    return std::unexpected{ "no extensions.VRMC_vrm" };
  }

  auto VRMC_vrm = extensions.at("VRMC_vrm");
  if (has(VRMC_vrm, "humanoid")) {
    auto& humanoid = VRMC_vrm.at("humanoid");
    if (has(humanoid, "humanBones")) {
      auto& humanBones = humanoid.at("humanBones");
      for (auto& kv : humanBones.items()) {
        if (auto bone =
              vrm::HumanBoneFromName(kv.key(), vrm::VrmVersion::_1_0)) {
          m_nodes[kv.value().at("node")]->Humanoid = NodeHumanoidInfo{
            .HumanBone = *bone,
          };
        } else {
          std::cout << kv.key() << std::endl;
        }
      }
    }
  }
  if (has(extensions, "VRMC_springBone")) {
    auto& springBone = extensions.at("VRMC_springBone");
    auto& springs = springBone.at("springs");
    for (auto& spring : springs) {
      auto solver = std::make_shared<vrm::SpringSolver>();
      std::shared_ptr<Node> head;
      for (auto& joint : spring.at("joints")) {
        int node_index = joint.at("node");
        auto tail = m_nodes[node_index];
        if (head) {
          float stiffness = joint.at("stiffness");
          float dragForce = joint.at("dragForce");
          solver->Add(head, tail->Transform.Translation, stiffness, dragForce);
        }
        head = tail;
      }
      m_springSolvers.push_back(solver);
    }
  }
  return true;
}

std::expected<bool, std::string>
Scene::AddIndices(int vertex_offset,
                  gltf::Mesh* mesh,
                  const nlohmann::json& prim,
                  const std::shared_ptr<gltf::Material>& material)
{
  if (has(prim, "indices")) {
    int accessor_index = prim.at("indices");
    auto accessor = m_gltf.Json.at("accessors").at(accessor_index);
    switch ((gltf::ComponentType)accessor.at("componentType")) {
      case gltf::ComponentType::UNSIGNED_BYTE: {
        if (auto span = m_gltf.accessor<uint8_t>(accessor_index)) {
          mesh->addSubmesh(vertex_offset, *span, material);
          return true;
        } else {
          return std::unexpected{ span.error() };
        }
      } break;
      case gltf::ComponentType::UNSIGNED_SHORT: {
        if (auto span = m_gltf.accessor<uint16_t>(accessor_index)) {
          mesh->addSubmesh(vertex_offset, *span, material);
          return true;
        } else {
          return std::unexpected{ span.error() };
        }
      } break;
      case gltf::ComponentType::UNSIGNED_INT: {
        if (auto span = m_gltf.accessor<uint32_t>(accessor_index)) {
          mesh->addSubmesh(vertex_offset, *span, material);
          return true;
        } else {
          return std::unexpected{ span.error() };
        }
      } break;
      default:
        return std::unexpected{ "invalid index type" };
    }
  } else {
    std::vector<uint32_t> indices;
    auto vertex_count = mesh->m_vertices.size();
    indices.reserve(vertex_count);
    for (int i = 0; i < vertex_count; ++i) {
      indices.push_back(i);
    }
    mesh->addSubmesh<uint32_t>(vertex_offset, indices, material);
    return true;
  }
}

void
Scene::Render(const RenderFunc& render, IGizmoDrawer* gizmo)
{
  // springbone
  for (auto& spring : m_springSolvers) {
    spring->Update(m_nextSpringDelta);
  }
  m_nextSpringDelta = {};

  if (m_expressions) {
    // VRM0 expression to morphTarget
    auto nodeToIndex = [nodes = m_nodes, expressions = m_expressions](
                         const std::shared_ptr<Node>& node) {
      for (uint32_t i = 0; i < nodes.size(); ++i) {
        if (node == nodes[i]) {
          return i;
        }
      }
      return (uint32_t)-1;
    };
    for (auto& [k, v] : m_expressions->EvalMorphTargetMap(nodeToIndex)) {
      auto& morph_node = m_nodes[k.NodeIndex];
      morph_node->Instance->weights[k.MorphIndex] = v;
    }
  }

  // skinning
  for (auto& node : m_nodes) {
    if (auto mesh_index = node->Mesh) {
      auto mesh = m_meshes[*mesh_index];

      // mesh animation
      std::span<DirectX::XMFLOAT4X4> skinningMatrices;

      // skinning
      if (auto skin = node->Skin) {
        skin->CurrentMatrices.resize(skin->BindMatrices.size());

        auto rootInverse = DirectX::XMMatrixIdentity();
        if (auto root_index = skin->Root) {
          rootInverse = DirectX::XMMatrixInverse(nullptr, node->WorldMatrix());
        }

        for (int i = 0; i < skin->Joints.size(); ++i) {
          auto node = m_nodes[skin->Joints[i]];
          auto m = skin->BindMatrices[i];
          DirectX::XMStoreFloat4x4(&skin->CurrentMatrices[i],
                                   DirectX::XMLoadFloat4x4(&m) *
                                     node->WorldMatrix() * rootInverse);
        }

        skinningMatrices = skin->CurrentMatrices;
      }

      // apply morphtarget & skinning
      node->Instance->applyMorphTargetAndSkinning(*mesh, skinningMatrices);
    }
  }
  DirectX::XMFLOAT4X4 m;
  for (auto& node : m_nodes) {
    if (auto mesh_index = node->Mesh) {
      auto mesh = m_meshes[*mesh_index];
      DirectX::XMStoreFloat4x4(&m, node->WorldMatrix());
      render(mesh, *node->Instance, &m._11);
    }
  }

  for (auto& spring : m_springSolvers) {
    spring->DrawGizmo(gizmo);
  }
}

void
Scene::Traverse(const EnterFunc& enter,
                const LeaveFunc& leave,
                const std::shared_ptr<gltf::Node>& node)
{
  if (node) {
    if (enter(node)) {
      for (auto& child : node->Children) {
        Traverse(enter, leave, child);
      }
      if (leave) {
        leave();
      }
    }
  } else {
    // root
    for (auto& child : m_roots) {
      Traverse(enter, leave, child);
    }
  }
}

void
Scene::TraverseJson(const EnterJson& enter,
                    const LeaveJson& leave,
                    nlohmann::json* item)
{
  if (!item) {
    // root
    m_jsonpath = "/";
    auto size = m_jsonpath.size();
    for (auto& kv : m_gltf.Json.items()) {
      m_jsonpath += kv.key();
      TraverseJson(enter, leave, &kv.value());
      m_jsonpath.resize(size);
    }
    return;
  }

  if (enter(*item, m_jsonpath)) {
    if (item->is_object()) {
      auto size = m_jsonpath.size();
      for (auto& kv : item->items()) {
        m_jsonpath.push_back(DELIMITER);
        m_jsonpath += kv.key();
        TraverseJson(enter, leave, &kv.value());
        m_jsonpath.resize(size);
      }
    } else if (item->is_array()) {
      auto size = m_jsonpath.size();
      for (int i = 0; i < item->size(); ++i) {
        std::stringstream ss;
        ss << i;
        auto str = ss.str();
        m_jsonpath.push_back(DELIMITER);
        m_jsonpath += str;
        TraverseJson(enter, leave, &(*item)[i]);
        m_jsonpath.resize(size);
      }
    }
    if (leave) {
      leave();
    }
  }
}

vrm::HumanPose
Scene::UpdateHumanPose()
{
  auto mult4 = [](const DirectX::XMVECTOR& q0,
                  const DirectX::XMVECTOR& q1,
                  const DirectX::XMVECTOR& q2,
                  const DirectX::XMVECTOR& q3) -> DirectX::XMVECTOR {
    return DirectX::XMQuaternionMultiply(
      DirectX::XMQuaternionMultiply(DirectX::XMQuaternionMultiply(q0, q1), q2),
      q3);
  };

  // retarget human pose
  m_humanBoneMap.clear();
  m_rotations.clear();
  for (auto& node : m_nodes) {
    if (auto humanoid = node->Humanoid) {
      m_humanBoneMap.push_back(humanoid->HumanBone);
      if (m_humanBoneMap.back() == vrm::HumanBones::hips) {
        // delta move
        DirectX::XMStoreFloat3(
          &m_pose.RootPosition,
          DirectX::XMVectorSubtract(
            DirectX::XMLoadFloat3(&node->WorldTransform.Translation),
            DirectX::XMLoadFloat3(&node->WorldInitialTransform.Translation)));
      }

      // retarget
      auto normalized =
        mult4(DirectX::XMQuaternionInverse(
                DirectX::XMLoadFloat4(&node->WorldInitialTransform.Rotation)),
              DirectX::XMLoadFloat4(&node->Transform.Rotation),
              DirectX::XMQuaternionInverse(
                DirectX::XMLoadFloat4(&node->InitialTransform.Rotation)),
              DirectX::XMLoadFloat4(&node->WorldInitialTransform.Rotation));

      m_rotations.push_back({});
      DirectX::XMStoreFloat4(&m_rotations.back(), normalized);
    }
  }
  m_pose.Bones = m_humanBoneMap;
  m_pose.Rotations = m_rotations;
  return m_pose;
}

void
Scene::SetHumanPose(const vrm::HumanPose& pose)
{
  assert(pose.Bones.size() == pose.Rotations.size());

  for (int i = 0; i < pose.Bones.size(); ++i) {
    if (auto node = GetBoneNode(pose.Bones[i])) {
      if (i == 0) {
        DirectX::XMStoreFloat3(
          &node->Transform.Translation,
          DirectX::XMVectorAdd(
            DirectX::XMLoadFloat3(&node->InitialTransform.Translation),
            DirectX::XMLoadFloat3(&pose.RootPosition)));
      }

      auto worldInitial =
        DirectX::XMLoadFloat4(&node->WorldInitialTransform.Rotation);
      auto q = DirectX::XMLoadFloat4(&pose.Rotations[i]);
      auto worldInitialInv = DirectX::XMQuaternionInverse(worldInitial);
      auto localInitial =
        DirectX::XMLoadFloat4(&node->InitialTransform.Rotation);

      // # retarget
      // normalized local rotation to unormalized hierarchy.
      DirectX::XMStoreFloat4(
        &node->Transform.Rotation,
        DirectX::XMQuaternionMultiply(
          DirectX::XMQuaternionMultiply(
            DirectX::XMQuaternionMultiply(worldInitial, q), worldInitialInv),
          localInitial));
    }
  }

  SyncHierarchy();
}

void
Scene::SyncHierarchy()
{
  // calc world
  auto enter = [](const std::shared_ptr<gltf::Node>& node) {
    node->CalcWorldMatrix();
    return true;
  };
  Traverse(enter, {});
}

std::shared_ptr<gltf::Node>
Scene::GetBoneNode(vrm::HumanBones bone)
{
  for (auto& node : m_nodes) {
    if (auto humanoid = node->Humanoid) {
      if (humanoid->HumanBone == bone) {
        return node;
      }
    }
  }
  return {};
}

BoundingBox
Scene::GetBoundingBox() const
{
  BoundingBox bb{};
  for (auto& node : m_nodes) {
    bb.Extend(node->WorldTransform.Translation);
    bb.Extend(node->WorldTransform.Translation);
    if (auto mesh_index = node->Mesh) {
      auto mesh_bb = m_meshes[*mesh_index]->GetBoundingBox();
      bb.Extend(dmath::transform(mesh_bb.Min, node->WorldMatrix()));
      bb.Extend(dmath::transform(mesh_bb.Max, node->WorldMatrix()));
    }
  }
  return bb;
}

}
