#pragma once
#include <boneskin/meshdeformer.h>
#include <filesystem>
#include <string>
#include <vrm/gltfroot.h>

class FbxLoader
{
  struct FbxLoaderImpl* m_impl;

public:
  FbxLoader();
  ~FbxLoader();
  std::tuple<std::shared_ptr<libvrm::GltfRoot>,
             std::shared_ptr<boneskin::MeshDeformer>>
  Load(const std::filesystem::path& path);
  std::string Error() const;
};
