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

namespace srht {
void
UpdateScene(const std::shared_ptr<gltf::Scene>& scene,
            std::vector<DirectX::XMFLOAT4X4>& instances,
            std::span<const uint8_t> data)
{
  BinaryReader r(data);

  auto magic = r.View(8);
  if (magic == "SRHTSKL1") {
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
}
