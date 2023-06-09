#pragma once
#include <filesystem>
#include <string>
#include <vrm/gltfroot.h>

class FbxLoader
{
  struct FbxLoaderImpl* m_impl;

public:
  FbxLoader();
  ~FbxLoader();
  std::shared_ptr<libvrm::GltfRoot> Load(const std::filesystem::path& path);
  std::string Error() const;
};
