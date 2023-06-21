#pragma once
#include "colorspace.h"
#include "renderpass.h"
#include <DirectXMath.h>
#include <filesystem>
#include <gltfjson.h>
#include <memory>

namespace grapho {
namespace gl3 {
class Texture;
class Cubemap;
}
}
namespace libvrm {
struct BaseMesh;
struct DeformedMesh;
struct NodeState;
class Image;
}
namespace cuber {
struct LineVertex;
};
namespace boneskin {
struct NodeMesh;
}

namespace glr {

struct Material;

enum class EnvCubemapTypes
{
  LOGL_IrradianceMap,
  LOGL_PrefilterMap,
  KHRONOS_LambertianEnvMap,
  KHRONOS_GGXEnvMap,
};

enum class EnvTextureTypes
{
  LOGL_BrdfLUT,
  KHRONOS_BrdfLUT,
};

struct RenderingEnv;

void
Initialize();
void
ClearBackBuffer(int width, int height);

void
RenderPasses(std::span<const RenderPass> passes,
             const RenderingEnv& camera,
             const gltfjson::Root& root,
             const gltfjson::Bin& bin,
             std::span<const boneskin::NodeMesh> meshNodes);

// clear current render target
void
ClearRendertarget(const RenderingEnv& camera);

// release all resource
void
Release();

void
ReleaseMaterial(int i);

std::shared_ptr<grapho::gl3::Texture>
GetOrCreateTexture(const gltfjson::Root& root,
                   const gltfjson::Bin& bin,
                   std::optional<uint32_t> texture,
                   glr::ColorSpace colorspace);

std::optional<uint32_t>
GetOrCreateTextureHandle(const gltfjson::Root& root,
                         const gltfjson::Bin& bin,
                         std::optional<uint32_t> texture,
                         ColorSpace colorspace);

void
RenderLine(const RenderingEnv& camera, std::span<const cuber::LineVertex> data);

// for local shader
void
SetShaderDir(const std::filesystem::path& path);

// for hot reload
// use relative path. pbr.{vs,fs}, unlit.{vs,fs}, mtoon.{vs,fs}
void
UpdateShader(const std::filesystem::path& path);

// for threejs shaderchunk
void
SetShaderChunkDir(const std::filesystem::path& path);

std::shared_ptr<grapho::gl3::Texture>
LoadPbr_LOGL(const std::filesystem::path& path);

bool
LoadPbr_Khronos(const std::filesystem::path& path);

bool
LoadPbr_Threejs(const std::filesystem::path& path);

std::shared_ptr<grapho::gl3::Texture>
GetEnvTexture(EnvTextureTypes type);

std::shared_ptr<grapho::gl3::Cubemap>
GetEnvCubemap(EnvCubemapTypes type);

void
RenderSkybox(const DirectX::XMFLOAT4X4& projection,
             const DirectX::XMFLOAT4X4& view);

std::vector<std::shared_ptr<Material>>&
MaterialMap();

std::shared_ptr<grapho::gl3::Texture>
CreateTexture(const std::shared_ptr<libvrm::Image>& image);

} // namespace
