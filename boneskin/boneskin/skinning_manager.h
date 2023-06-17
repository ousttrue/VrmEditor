#pragma once
#include "base_mesh.h"
#include "deformed_mesh.h"
#include "skin.h"

namespace boneskin {

class SkinningManager
{
  std::unordered_map<uint32_t, std::shared_ptr<DeformedMesh>> m_deformMap;
  std::unordered_map<uint32_t, std::shared_ptr<BaseMesh>> m_baseMap;
  std::unordered_map<uint32_t, std::shared_ptr<Skin>> m_skinMap;

  SkinningManager(){}

public:
  static SkinningManager& Instance()
  {
    static SkinningManager s_instance;
    return s_instance;
  }
  SkinningManager(const SkinningManager&) = delete;
  SkinningManager& operator=(const SkinningManager&) = delete;

  void Release()
  {
    m_baseMap.clear();
    m_deformMap.clear();
    m_skinMap.clear();
  }

  std::shared_ptr<BaseMesh> GetOrCreateBaseMesh(const gltfjson::Root& root,
                                                const gltfjson::Bin& bin,
                                                std::optional<uint32_t> mesh);

  std::shared_ptr<DeformedMesh> GetOrCreateDeformedMesh(
    uint32_t mesh,
    const std::shared_ptr<BaseMesh>& baseMesh);

  std::shared_ptr<Skin> GetOrCreaeSkin(const gltfjson::Root& root,
                                       const gltfjson::Bin& bin,
                                       std::optional<uint32_t> skinId);
};

} // namespace
