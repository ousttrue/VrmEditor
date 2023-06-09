#pragma once
#include <filesystem>
#include <string>

class FbxLoader
{
  struct FbxLoaderImpl* m_impl;

public:
  FbxLoader();
  ~FbxLoader();
  bool Load(const std::filesystem::path& path);
  std::string Error() const;
};
