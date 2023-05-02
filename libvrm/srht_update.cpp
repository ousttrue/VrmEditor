#include "vrm/srht_update.h"
#include "vrm/scene.h"
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

namespace libvrm {

namespace srht {
void
UpdateScene(const std::shared_ptr<gltf::Scene>& scene,
            std::span<const uint8_t> data)
{
  BinaryReader r(data);

  auto magic = r.View(8);
  if (magic == "SRHTSKL1") {
    scene->Clear();

    auto header = r.Get<SkeletonHeader>();
    std::vector<JointDefinition> joints(header.jointCount);
    r.CopyTo(std::span(joints));
    std::vector<DirectX::XMFLOAT4> rotations(header.jointCount);
    // initial rotations
    if ((header.flags & SkeletonFlags::HAS_INITIAL_ROTATION) !=
        SkeletonFlags::NONE) {
      rotations.resize(header.jointCount);
      r.CopyTo(std::span(rotations));
    } else {
      std::fill(
        rotations.begin(), rotations.end(), DirectX::XMFLOAT4{ 0, 0, 0, 1 });
    }

    // create nodes
    for (int i = 0; i < joints.size(); ++i) {
      char buf[256];
      snprintf(buf, sizeof(buf), "%d", i);
      auto ptr = std::make_shared<gltf::Node>(buf);
      auto& joint = joints[i];
      ptr->InitialTransform.Translation.x = joint.xFromParent;
      ptr->InitialTransform.Translation.y = joint.yFromParent;
      ptr->InitialTransform.Translation.z = joint.zFromParent;
      auto& rotation = rotations[i];
      ptr->InitialTransform.Rotation = rotation;

      if (auto vrm_bone = ToVrmBone((HumanoidBones)joint.boneType)) {
        ptr->Humanoid = *vrm_bone;
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
    scene->InitializeNodes();
    scene->RaiseSceneUpdated();
  } else if (magic == "SRHTFRM1") {
    if (scene->m_roots.size()) {
      auto header = r.Get<FrameHeader>();
      auto root = scene->m_roots[0];
      auto humanoid = root->Humanoid;
      if (!humanoid || *humanoid == vrm::HumanBones::hips) {
        // only move non humanoid or hips
        // root->Transform.Translation.x = header.x;
        // root->Transform.Translation.y = header.y;
        // root->Transform.Translation.z = header.z;
      }
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
        // scene->m_nodes[i]->Transform.Rotation = rotations[i];
      }
      // scene->m_roots[0]->CalcWorldMatrix(true);
      // scene->RaiseSceneUpdated();
    }
  } else {
    assert(false);
  }
}

void
MakeSkeleton(uint32_t skeletonId,
             const std::shared_ptr<gltf::Node>& root,
             std::vector<uint8_t>& out)
{
}

void
MakeFrame(uint32_t skeletonId,
          const std::shared_ptr<gltf::Node>& root,
          std::vector<uint8_t>& out)
{
}

}
}
