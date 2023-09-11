#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace glr {

// enum class ShaderTypes
// {
//   Error,  // fallback
//   MToon1, // VRMC_materials_mtton
//   MToon0,
//   Unlit, // KHR_materials_unlit
//   Pbr,   // glTF default
//
//   Shadow,
// };

struct ShaderSource
{
  std::filesystem::path Path;
  std::string Source;
  std::vector<std::filesystem::path> Includes;

  void Reload(const std::filesystem::path& dir,
              const std::filesystem::path& chunkDir);
};

// manage shader source for hot reload
class ShaderSourceManager
{
  struct ShaderSourceManagerImpl* m_impl = nullptr;

public:
  ShaderSourceManager();
  ~ShaderSourceManager();
  void SetShaderDir(const std::filesystem::path& path);
  void SetShaderChunkDir(const std::filesystem::path& path);
  std::shared_ptr<ShaderSource> Get(const std::string& filename);
  std::vector<std::filesystem::path> UpdateShader(const std::filesystem::path& path);
};

}
