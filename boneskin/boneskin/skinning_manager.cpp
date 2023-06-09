#include "skinning_manager.h"
#include <vrm/gltfroot.h>
#include <vrm/node_state.h>

namespace boneskin {

static std::expected<std::shared_ptr<boneskin::Skin>, std::string>
ParseSkin(const gltfjson::Root& root, const gltfjson::Bin& bin, uint32_t i)
{
  auto skin = root.Skins[i];
  auto ptr = std::make_shared<boneskin::Skin>();
  ptr->Name = gltfjson::from_u8(skin.NameString());
  for (auto joint : skin.Joints) {
    ptr->Joints.push_back(joint);
  }

  std::span<const DirectX::XMFLOAT4X4> matrices;
  if (auto accessor = bin.GetAccessorBytes<DirectX::XMFLOAT4X4>(
        root, *skin.InverseBindMatricesId())) {
    matrices = *accessor;
  } else {
    return std::unexpected{ accessor.error() };
  }
  std::vector<DirectX::XMFLOAT4X4> copy;
  ptr->BindMatrices.assign(matrices.begin(), matrices.end());

  assert(ptr->Joints.size() == ptr->BindMatrices.size());

  ptr->Root = skin.SkeletonId();
  return ptr;
}

static std::expected<bool, std::string>
AddIndices(const gltfjson::Root& root,
           const gltfjson::Bin& bin,
           int vertex_offset,
           boneskin::BaseMesh* mesh,
           const gltfjson::MeshPrimitive& prim)
{
  if (auto indices = prim.IndicesId()) {
    auto accessor_index = (uint32_t)*indices;
    auto accessor = root.Accessors[accessor_index];
    switch ((gltfjson::ComponentTypes)*accessor.ComponentType()) {
      case gltfjson::ComponentTypes::UNSIGNED_BYTE: {
        if (auto span = bin.GetAccessorBytes<uint8_t>(root, accessor_index)) {
          mesh->addSubmesh(vertex_offset, *span, prim.MaterialId());
          return true;
        } else {
          return std::unexpected{ span.error() };
        }
      } break;
      case gltfjson::ComponentTypes::UNSIGNED_SHORT: {
        if (auto span = bin.GetAccessorBytes<uint16_t>(root, accessor_index)) {
          mesh->addSubmesh(vertex_offset, *span, prim.MaterialId());
          return true;
        } else {
          return std::unexpected{ span.error() };
        }
      } break;
      case gltfjson::ComponentTypes::UNSIGNED_INT: {
        if (auto span = bin.GetAccessorBytes<uint32_t>(root, accessor_index)) {
          mesh->addSubmesh(vertex_offset, *span, prim.MaterialId());
          return true;
        } else {
          return std::unexpected{ span.error() };
        }
      } break;
      default:
        return std::unexpected{ "invalid index type" };
    }
  } else {
    std::vector<uint32_t> indexList;
    auto vertex_count = mesh->m_vertices.size();
    indexList.reserve(vertex_count);
    for (int i = 0; i < vertex_count; ++i) {
      indexList.push_back(i);
    }
    mesh->addSubmesh<uint32_t>(vertex_offset, indexList, prim.MaterialId());
    return true;
  }
}

static std::expected<std::shared_ptr<boneskin::BaseMesh>, std::string>
ParseMesh(const gltfjson::Root& root, const gltfjson::Bin& bin, int meshIndex)
{
  auto mesh = root.Meshes[meshIndex];
  auto ptr = std::make_shared<boneskin::BaseMesh>();
  ptr->Name = mesh.NameString();
  std::optional<gltfjson::MeshPrimitiveAttributes> lastAtributes;

  for (auto prim : mesh.Primitives) {
    if (prim.Attributes() == lastAtributes) {
      // for vrm shared vertex buffer
      if (auto expected = AddIndices(root, bin, 0, ptr.get(), prim)) {
        // OK
      } else {
        return std::unexpected{ expected.error() };
      }
    } else {
      // extend vertex buffer
      uint32_t offset = 0;
      if (auto positions =
            bin.GetAccessorBlock(root, *prim.Attributes()->POSITION_Id())) {
        offset = ptr->AddPosition(*positions);
      } else {
        return std::unexpected{ positions.error() };
      }

      if (auto normal = prim.Attributes()->NORMAL_Id()) {
        if (auto normals = bin.GetAccessorBlock(root, *normal)) {
          ptr->SetNormal(offset, *normals);
        } else {
          return std::unexpected{ normals.error() };
        }
      }

      if (auto tex0 = prim.Attributes()->TEXCOORD_0_Id()) {
        if (auto uv = bin.GetAccessorBlock(root, *tex0)) {
          ptr->SetUv(offset, *uv);
        } else {
          return std::unexpected{ uv.error() };
        }
      }

      auto joints0 = prim.Attributes()->JOINTS_0_Id();
      auto weights0 = prim.Attributes()->WEIGHTS_0_Id();
      if (joints0 && weights0) {
        // skinning
        if (auto joints = bin.GetAccessorBlock(root, *joints0)) {
          if (auto weights = bin.GetAccessorBlock(root, *weights0)) {
            switch (joints->ItemSize) {
              case 4:
                ptr->SetBoneSkinning<boneskin::byte4>(
                  offset, *joints, *weights);
                break;

              case 8:
                ptr->SetBoneSkinning<boneskin::ushort4>(
                  offset, *joints, *weights);
                break;

              default:
                assert(false);
                return std::unexpected{ "JOINTS_0: not implemented" };
            }
          }
        }
      }

      // extend morph target
      {
        auto& targets = prim.Targets;
        for (int i = 0; i < targets.size(); ++i) {
          auto target = targets[i];
          auto morph = ptr->getOrCreateMorphTarget(i);
          // std::cout << target << std::endl;
          std::span<const DirectX::XMFLOAT3> positions;
          if (auto accessor = bin.GetAccessorBytes<DirectX::XMFLOAT3>(
                root, *target.POSITION_Id())) {
            positions = *accessor;
          } else {
            return std::unexpected{ accessor.error() };
          }
          // if (scene->m_type == ModelType::Vrm0) {
          //   std::vector<DirectX::XMFLOAT3> copy;
          //   copy.reserve(positions.size());
          //   for (auto& p : positions) {
          //     copy.push_back({ -p.x, p.y, -p.z });
          //   }
          //   positions = copy;
          // }
          /*auto morphOffset =*/morph->addPosition(positions);
        }
      }

      // extend indices and add vertex offset
      if (auto expected = AddIndices(root, bin, offset, ptr.get(), prim)) {
        // OK
      } else {
        return std::unexpected{ expected.error() };
      }
    }

    // find morph target name
    // primitive.extras.targetNames
    // if (has(prim, "extras")) {
    //   auto& extras = prim.at("extras");
    //   if (has(extras, "targetNames")) {
    //     auto& names = extras.at("targetNames");
    //     // std::cout << names << std::endl;
    //     for (int i = 0; i < names.size(); ++i) {
    //       ptr->getOrCreateMorphTarget(i)->Name = names[i];
    //     }
    //   }
    // }

    lastAtributes = *prim.Attributes();
  }

  // find morph target name
  // mesh.extras.targetNames
  // if (has(mesh, "extras")) {
  //   auto& extras = mesh.at("extras");
  //   if (has(extras, "targetNames")) {
  //     auto& names = extras.at("targetNames");
  //     // std::cout << names << std::endl;
  //     for (int i = 0; i < names.size(); ++i) {
  //       ptr->getOrCreateMorphTarget(i)->Name = names[i];
  //     }
  //   }
  // }

  return ptr;
}

std::shared_ptr<BaseMesh>
SkinningManager::GetOrCreateBaseMesh(const gltfjson::Root& root,
                                     const gltfjson::Bin& bin,
                                     std::optional<uint32_t> mesh)
{
  if (!mesh) {
    return {};
  }

  auto found = m_baseMap.find(*mesh);
  if (found != m_baseMap.end()) {
    return found->second;
  }

  if (auto base = ParseMesh(root, bin, *mesh)) {
    m_baseMap.insert({ *mesh, *base });
    return *base;
  } else {
    return {};
  }
}

std::shared_ptr<DeformedMesh>
SkinningManager::GetOrCreateDeformedMesh(
  uint32_t mesh,
  const std::shared_ptr<BaseMesh>& baseMesh)
{
  auto found = m_deformMap.find(mesh);
  if (found != m_deformMap.end()) {
    return found->second;
  }

  auto runtime = std::make_shared<DeformedMesh>(baseMesh);
  m_deformMap.insert({ mesh, runtime });
  return runtime;
}

std::shared_ptr<Skin>
SkinningManager::GetOrCreaeSkin(const gltfjson::Root& root,
                                const gltfjson::Bin& bin,
                                std::optional<uint32_t> skinId)
{
  if (!skinId) {
    return {};
  }

  auto found = m_skinMap.find(*skinId);
  if (found != m_skinMap.end()) {
    return found->second;
  }

  if (auto skin = ParseSkin(root, bin, *skinId)) {
    m_skinMap.insert({ *skinId, *skin });
    return *skin;
  } else {
    return {};
  }
}

static void
ApplyMorphTarget(DeformedMesh& deformed,
                 const BaseMesh& mesh,
                 const std::unordered_map<uint32_t, float>& morphMap)
{
  deformed.Vertices.assign(mesh.m_vertices.begin(), mesh.m_vertices.end());
  if (morphMap.size()) {
    for (int j = 0; j < mesh.m_morphTargets.size(); ++j) {
      auto& morphtarget = mesh.m_morphTargets[j];
      auto found = morphMap.find(j);
      if (found != morphMap.end()) {
        auto weight = found->second;
        if (weight > 0) {
          for (int i = 0; i < mesh.m_vertices.size(); ++i) {
            // auto v = mesh.m_vertices[i];
            deformed.Vertices[i].Position +=
              morphtarget->Vertices[i].position * weight;
          }
        }
      }
    }
  }
}

static void
SkinningVertex(Vertex* dst,
               // const Vertex& src,
               const DirectX::XMVECTOR& pos,
               const DirectX::XMVECTOR& normal,
               float w,
               std::span<const DirectX::XMFLOAT4X4> matrices,
               uint16_t matrixIndex)
{
  if (w > 0) {
    if (matrixIndex < matrices.size()) {
      DirectX::XMFLOAT3 store;
      {
        auto newPos = DirectX::XMVector3Transform(
          pos, DirectX::XMLoadFloat4x4(&matrices[matrixIndex]));
        DirectX::XMStoreFloat3(&store, newPos);
        dst->Position += (store * w);
      }
      {
        auto newNormal = DirectX::XMVector3Transform(
          normal, DirectX::XMLoadFloat4x4(&matrices[matrixIndex]));
        DirectX::XMStoreFloat3(&store, newNormal);
        dst->Normal += (store * w);
      }
    } else {
      // error
    }
  }
}

static void
ApplySkinning(DeformedMesh& deformed,
              std::span<const JointBinding> bindings,
              std::span<const DirectX::XMFLOAT4X4> skinningMatrices)
{
  if (skinningMatrices.size()) {
    for (int i = 0; i < deformed.Vertices.size(); ++i) {
      auto src = deformed.Vertices[i];
      auto pos = DirectX::XMLoadFloat3(&src.Position);
      auto normal = DirectX::XMLoadFloat3(&src.Normal);
      auto& dst = deformed.Vertices[i];
      dst.Position = { 0, 0, 0 };
      dst.Normal = { 0, 0, 0 };
      auto binding = bindings[i];
      if (auto w = binding.Weights.x)
        SkinningVertex(
          &dst, pos, normal, w, skinningMatrices, binding.Joints.X);
      if (auto w = binding.Weights.y)
        SkinningVertex(
          &dst, pos, normal, w, skinningMatrices, binding.Joints.Y);
      if (auto w = binding.Weights.z)
        SkinningVertex(
          &dst, pos, normal, w, skinningMatrices, binding.Joints.Z);
      if (auto w = binding.Weights.w)
        SkinningVertex(
          &dst, pos, normal, w, skinningMatrices, binding.Joints.W);
    }
  }
}

std::span<const NodeMesh>
SkinningManager::ProcessSkin(const gltfjson::Root& root,
                             const gltfjson::Bin& bin,
                             std::span<const libvrm::NodeState> drawables)
{
  assert(root.Nodes.size() == drawables.size());
  m_meshNodes.clear();

  for (uint32_t i = 0; i < drawables.size(); ++i) {
    auto gltfNode = root.Nodes[i];
    auto& drawable = drawables[i];
    if (auto meshId = gltfNode.MeshId()) {
      m_meshNodes.push_back({ i, *meshId, drawable.Matrix });
      if (auto baseMesh =
            boneskin::SkinningManager::Instance().GetOrCreateBaseMesh(
              root, bin, meshId)) {

        auto deformed =
          boneskin::SkinningManager::Instance().GetOrCreateDeformedMesh(
            *meshId, baseMesh);
        if (deformed->Vertices.size()) {
          // morph
          ApplyMorphTarget(*deformed, *baseMesh, drawable.MorphMap);

          if (auto skin = boneskin::SkinningManager::Instance().GetOrCreaeSkin(
                root, bin, gltfNode.SkinId())) {
            // update skinnning
            skin->CurrentMatrices.resize(skin->BindMatrices.size());

            auto rootInverse = DirectX::XMMatrixIdentity();
            if (auto root_index = skin->Root) {
              rootInverse = DirectX::XMMatrixInverse(
                nullptr, DirectX::XMLoadFloat4x4(&drawable.Matrix));
            }

            for (int i = 0; i < skin->Joints.size(); ++i) {
              auto m = skin->BindMatrices[i];
              DirectX::XMStoreFloat4x4(
                &skin->CurrentMatrices[i],
                DirectX::XMLoadFloat4x4(&m) *
                  DirectX::XMLoadFloat4x4(&drawables[skin->Joints[i]].Matrix) *
                  rootInverse);
            }

            ApplySkinning(
              *deformed, baseMesh->m_bindings, skin->CurrentMatrices);
          }
        }
      }
    }
  }

  return m_meshNodes;
}

} // namespace
