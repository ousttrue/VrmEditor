#include "vrm/srht_update.h"
#include "vrm/srht.h"
#include <algorithm>

struct BinaryReader
{
  std::span<const uint8_t> m_data;
  size_t m_pos = 0;
  BinaryReader(std::span<const uint8_t> data)
    : m_data(data)
  {
  }

  std::span<const uint8_t> Bytes(size_t size)
  {
    auto data = m_data.data() + m_pos;
    m_pos += size;
    return std::span(data, size);
  }

  std::string_view View(size_t size)
  {
    auto data = (const char*)m_data.data() + m_pos;
    m_pos += size;
    return std::string_view(data, data + size);
  }

  template<typename T>
  T Get()
  {
    auto data = Bytes(sizeof(T));
    return *((T*)data.data());
  }

  template<typename T>
  void CopyTo(std::span<T> data)
  {
    auto p = m_data.data() + m_pos;
    auto byteSize = data.size() * sizeof(T);
    std::copy(p, p + byteSize, (uint8_t*)data.data());
    m_pos += byteSize;
  }
};

static std::optional<vrm::HumanBones>
ToVrmBone(srht::HumanoidBones src)
{
  switch (src) {
      // body: 6
    case srht::HumanoidBones::HIPS:
      return vrm::HumanBones::hips;
    case srht::HumanoidBones::SPINE:
      return vrm::HumanBones::spine;
    case srht::HumanoidBones::CHEST:
      return vrm::HumanBones::chest;
    case srht::HumanoidBones::UPPERCHEST:
      return vrm::HumanBones::upperChest;
    case srht::HumanoidBones::NECK:
      return vrm::HumanBones::neck;
    case srht::HumanoidBones::HEAD:
      return vrm::HumanBones::head;
      // legs: 4 x 2
    case srht::HumanoidBones::LEFT_UPPERLEG:
      return vrm::HumanBones::leftUpperLeg;
    case srht::HumanoidBones::LEFT_LOWERLEG:
      return vrm::HumanBones::leftLowerLeg;
    case srht::HumanoidBones::LEFT_FOOT:
      return vrm::HumanBones::leftFoot;
    case srht::HumanoidBones::LEFT_TOES:
      return vrm::HumanBones::leftToes;
    case srht::HumanoidBones::RIGHT_UPPERLEG:
      return vrm::HumanBones::rightUpperLeg;
    case srht::HumanoidBones::RIGHT_LOWERLEG:
      return vrm::HumanBones::rightLowerLeg;
    case srht::HumanoidBones::RIGHT_FOOT:
      return vrm::HumanBones::rightFoot;
    case srht::HumanoidBones::RIGHT_TOES:
      return vrm::HumanBones::rightToes;
      // arms: 4 x 2
    case srht::HumanoidBones::LEFT_SHOULDER:
      return vrm::HumanBones::leftShoulder;
    case srht::HumanoidBones::LEFT_UPPERARM:
      return vrm::HumanBones::leftUpperArm;
    case srht::HumanoidBones::LEFT_LOWERARM:
      return vrm::HumanBones::leftLowerArm;
    case srht::HumanoidBones::LEFT_HAND:
      return vrm::HumanBones::leftHand;
    case srht::HumanoidBones::RIGHT_SHOULDER:
      return vrm::HumanBones::rightShoulder;
    case srht::HumanoidBones::RIGHT_UPPERARM:
      return vrm::HumanBones::rightUpperArm;
    case srht::HumanoidBones::RIGHT_LOWERARM:
      return vrm::HumanBones::rightLowerArm;
    case srht::HumanoidBones::RIGHT_HAND:
      return vrm::HumanBones::rightHand;
      // fingers: 3 x 5 x 2
    case srht::HumanoidBones::LEFT_THUMB_METACARPAL:
      return vrm::HumanBones::leftThumbMetacarpal;
    case srht::HumanoidBones::LEFT_THUMB_PROXIMAL:
      return vrm::HumanBones::leftThumbProximal;
    case srht::HumanoidBones::LEFT_THUMB_DISTAL:
      return vrm::HumanBones::leftThumbDistal;
    case srht::HumanoidBones::LEFT_INDEX_PROXIMAL:
      return vrm::HumanBones::leftIndexProximal;
    case srht::HumanoidBones::LEFT_INDEX_INTERMEDIATE:
      return vrm::HumanBones::leftIndexIntermediate;
    case srht::HumanoidBones::LEFT_INDEX_DISTAL:
      return vrm::HumanBones::leftIndexDistal;
    case srht::HumanoidBones::LEFT_MIDDLE_PROXIMAL:
      return vrm::HumanBones::leftMiddleProximal;
    case srht::HumanoidBones::LEFT_MIDDLE_INTERMEDIATE:
      return vrm::HumanBones::leftMiddleIntermediate;
    case srht::HumanoidBones::LEFT_MIDDLE_DISTAL:
      return vrm::HumanBones::leftMiddleDistal;
    case srht::HumanoidBones::LEFT_RING_PROXIMAL:
      return vrm::HumanBones::leftRingProximal;
    case srht::HumanoidBones::LEFT_RING_INTERMEDIATE:
      return vrm::HumanBones::leftRingIntermediate;
    case srht::HumanoidBones::LEFT_RING_DISTAL:
      return vrm::HumanBones::leftRingDistal;
    case srht::HumanoidBones::LEFT_LITTLE_PROXIMAL:
      return vrm::HumanBones::leftLittleProximal;
    case srht::HumanoidBones::LEFT_LITTLE_INTERMEDIATE:
      return vrm::HumanBones::leftLittleIntermediate;
    case srht::HumanoidBones::LEFT_LITTLE_DISTAL:
      return vrm::HumanBones::leftLittleDistal;
    case srht::HumanoidBones::RIGHT_THUMB_METACARPAL:
      return vrm::HumanBones::rightThumbMetacarpal;
    case srht::HumanoidBones::RIGHT_THUMB_PROXIMAL:
      return vrm::HumanBones::rightThumbProximal;
    case srht::HumanoidBones::RIGHT_THUMB_DISTAL:
      return vrm::HumanBones::rightThumbMetacarpal;
    case srht::HumanoidBones::RIGHT_INDEX_PROXIMAL:
      return vrm::HumanBones::rightIndexProximal;
    case srht::HumanoidBones::RIGHT_INDEX_INTERMEDIATE:
      return vrm::HumanBones::rightIndexIntermediate;
    case srht::HumanoidBones::RIGHT_INDEX_DISTAL:
      return vrm::HumanBones::rightIndexDistal;
    case srht::HumanoidBones::RIGHT_MIDDLE_PROXIMAL:
      return vrm::HumanBones::rightMiddleProximal;
    case srht::HumanoidBones::RIGHT_MIDDLE_INTERMEDIATE:
      return vrm::HumanBones::rightMiddleIntermediate;
    case srht::HumanoidBones::RIGHT_MIDDLE_DISTAL:
      return vrm::HumanBones::rightMiddleDistal;
    case srht::HumanoidBones::RIGHT_RING_PROXIMAL:
      return vrm::HumanBones::rightRingProximal;
    case srht::HumanoidBones::RIGHT_RING_INTERMEDIATE:
      return vrm::HumanBones::rightRingIntermediate;
    case srht::HumanoidBones::RIGHT_RING_DISTAL:
      return vrm::HumanBones::rightRingDistal;
    case srht::HumanoidBones::RIGHT_LITTLE_PROXIMAL:
      return vrm::HumanBones::rightLittleProximal;
    case srht::HumanoidBones::RIGHT_LITTLE_INTERMEDIATE:
      return vrm::HumanBones::rightLittleIntermediate;
    case srht::HumanoidBones::RIGHT_LITTLE_DISTAL:
      return vrm::HumanBones::rightLittleDistal;
  }

  return {};
}

namespace srht {
void
UpdateScene(const std::shared_ptr<gltf::Scene>& scene,
            std::vector<DirectX::XMFLOAT4X4>& instances,
            std::span<const uint8_t> data)
{
  BinaryReader r(data);

  auto magic = r.View(8);
  if (magic == "SRHTSKL1") {
    scene->Clear();

    auto header = r.Get<SkeletonHeader>();
    std::vector<JointDefinition> joints(header.jointCount);
    r.CopyTo(std::span(joints));

    // create nodes
    int i = 0;
    for (auto& joint : joints) {
      char buf[256];
      snprintf(buf, sizeof(buf), "%d", i++);
      auto ptr = std::make_shared<gltf::Node>(buf);
      ptr->Transform.Translation.x = joint.xFromParent;
      ptr->Transform.Translation.y = joint.yFromParent;
      ptr->Transform.Translation.z = joint.zFromParent;
      if (auto vrm_bone = ToVrmBone((HumanoidBones)joint.boneType)) {
        ptr->Humanoid = gltf::NodeHumanoidInfo{
          .HumanBone = *vrm_bone,
        };
      }
      scene->m_nodes.push_back(ptr);
    }
    for (int i = 0; i < joints.size(); ++i) {
      auto joint = joints[i];
      auto ptr = scene->m_nodes[i];
      if (joints[i].parentBoneIndex == USHRT_MAX) {
        scene->m_roots.push_back(ptr);
      } else {
        auto parent = scene->m_nodes[joint.parentBoneIndex];
        gltf::Node::AddChild(parent, ptr);
      }
    }
    scene->InitNodes();
    scene->m_roots[0]->UpdateShapeInstanceRecursive(DirectX::XMMatrixIdentity(),
                                                    instances);

  } else if (magic == "SRHTFRM1") {
    if (scene->m_roots.size()) {
      auto header = r.Get<FrameHeader>();
      scene->m_roots[0]->Transform.Translation.x = header.x;
      scene->m_roots[0]->Transform.Translation.y = header.y;
      scene->m_roots[0]->Transform.Translation.z = header.z;
      std::vector<DirectX::XMFLOAT4> rotations(scene->m_nodes.size());
      if ((int)header.flags & (int)FrameFlags::USE_QUAT32) {
        std::vector<uint32_t> packs(scene->m_nodes.size());
        r.CopyTo(std::span(packs));
        for (int i = 0; i < packs.size(); ++i) {
          quat_packer::Unpack(packs[i], &rotations[i].x);
        }
      } else {
        r.CopyTo(std::span(rotations));
      }
      for (int i = 0; i < rotations.size(); ++i) {
        scene->m_nodes[i]->Transform.Rotation = rotations[i];
      }
      scene->m_roots[0]->CalcWorldMatrix(true);
      scene->RaiseSceneUpdated();
    }
  } else {
    assert(false);
  }
}

void
MakeSkeleton(uint32_t skeletonId,
             const std::shared_ptr<gltf::Node>& root,
             std::vector<uint8_t>& out)
{}

void
MakeFrame(uint32_t skeletonId,
          const std::shared_ptr<gltf::Node>& root,
          std::vector<uint8_t>& out)
{}

}
