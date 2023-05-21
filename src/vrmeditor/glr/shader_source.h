#pragma once
#include <filesystem>
#include <string>

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
  void UpdateShader(const std::filesystem::path& path);
};

}
