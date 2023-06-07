#pragma once
#include "colorspace.h"
// #include "docks/gui.h"
#include "renderpass.h"
#include <cuber/mesh.h>
#include <filesystem>
#include <gltfjson.h>
#include <memory>

// namespace libvrm {
// namespace gltf {
// struct Mesh;
// class Image;
// struct Texture;
// }
// }
namespace grapho {
namespace gl3 {
class Texture;
class Cubemap;
}
}
namespace libvrm {
struct BaseMesh;
struct DeformedMesh;
struct DrawItem;
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
RenderPasses(std::span<const RenderPass> passes,
             const RenderingEnv& camera,
             const gltfjson::Root& root,
             const gltfjson::Bin& bin,
             std::span<const libvrm::DrawItem> drawables);

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

bool
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

std::shared_ptr<Material>
GetMaterial(uint32_t index);

} // namespace
