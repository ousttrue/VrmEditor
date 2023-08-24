#pragma once
#include <vrm/gltfroot.h>
#include <filesystem>

class Lighting
{
  struct LightingImpl* m_impl;

public:
  Lighting();
  ~Lighting();
  bool LoadHdr(const std::filesystem::path &path);
  void SetGltf(const std::shared_ptr<libvrm::GltfRoot> &root);
  void ShowGui();
};
