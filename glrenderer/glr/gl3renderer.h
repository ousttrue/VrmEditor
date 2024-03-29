#pragma once
#include "colorspace.h"
#include "renderpass.h"
#include <DirectXMath.h>
#include <boneskin/base_mesh.h>
#include <filesystem>
#include <gltfjson.h>
#include <memory>

namespace grapho {
namespace camera {
struct Camera;
}
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

namespace glr {

struct Material;

struct MeshDrawInfo
{
  uint32_t MeshId;
  DirectX::XMFLOAT4X4 Matrix;
  std::shared_ptr<boneskin::BaseMesh> BaseMesh;
  std::span<const boneskin::Vertex> Vertices;
};

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
             const grapho::camera::Camera& camera,
             const RenderingEnv& env,
             const gltfjson::Root& root,
             const gltfjson::Bin& bin,
             const MeshDrawInfo& draw);

// clear current render target
void
ClearRendertarget(const grapho::camera::Camera& camera,
                  const RenderingEnv& env);

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

std::optional<uint32_t>
GetOrCreateTextureHandle(const std::shared_ptr<libvrm::Image>& image,
                         ColorSpace colorspace);

std::shared_ptr<libvrm::Image>
GetOrCreateImage(const gltfjson::Root& root,
                 const gltfjson::Bin& bin,
                 std::optional<uint32_t> image);

void
RenderLine(const grapho::camera::Camera& camera,
           std::span<const cuber::LineVertex> data);

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
CreateTexture(const libvrm::Image& image);

} // namespace
