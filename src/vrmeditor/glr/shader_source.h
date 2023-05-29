#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace glr {

enum class ShaderTypes
{
  Error,  // fallback
  MToon1, // VRMC_materials_mtton
  MToon0,
  Unlit, // KHR_materials_unlit
  Pbr,   // glTF default

  Shadow,
};

struct ShaderFileName
{
  std::string Vert;
  std::string Frag;
};

struct ShaderExpanded
{
  std::u8string_view Vert;
  std::u8string_view Frag;
};

// manage shader source for hot reload
class ShaderSourceManager
{
  struct ShaderSourceManagerImpl* m_impl = nullptr;

public:
  ShaderSourceManager();
  ~ShaderSourceManager();
  void Register(ShaderTypes type, const ShaderFileName& files);
  ShaderExpanded Get(ShaderTypes type) const;
  void SetShaderDir(const std::filesystem::path& path);
  void SetShaderChunkDir(const std::filesystem::path& path);
  std::vector<ShaderTypes> UpdateShader(const std::filesystem::path& path);
};

}
