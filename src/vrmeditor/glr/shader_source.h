#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace glr {

// manage shader source for hot reload
class ShaderSourceManager
{
  struct ShaderSourceManagerImpl* m_impl = nullptr;

public:
  ShaderSourceManager();
  ~ShaderSourceManager();
  std::u8string_view Get(const std::filesystem::path& path) const;
  void SetShaderDir(const std::filesystem::path& path);
  std::vector<std::filesystem::path> UpdateShader(
    const std::filesystem::path& path);
};

}
