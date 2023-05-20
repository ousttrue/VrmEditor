#pragma once
#include "docks/gui.h"
#include "renderpass.h"
#include <cuber/mesh.h>
#include <filesystem>
#include <memory>
#include <vrm/colorspace.h>

namespace libvrm {
namespace gltf {
struct Mesh;
class Image;
struct Texture;
}
}
namespace grapho {
namespace gl3 {
class Texture;
}
}

namespace runtimescene {
struct BaseMesh;
struct DeformedMesh;
}

namespace gltfjson {
namespace format {
struct Root;
struct Bin;
}
}

namespace glr {
struct RenderingEnv;

void
Render(RenderPass pass,
       const RenderingEnv& camera,
       const gltfjson::format::Root& root,
       const gltfjson::format::Bin& bin,
       const std::shared_ptr<runtimescene::BaseMesh>& mesh,
       const runtimescene::DeformedMesh& instance,
       const DirectX::XMFLOAT4X4& m);

// clear current render target
void
ClearRendertarget(const RenderingEnv& camera);

// release all resource
void
Shutdown();

// resource viewer
void
CreateDock(const AddDockFunc& addDock, std::string_view title);

std::shared_ptr<grapho::gl3::Texture>
GetOrCreateTexture(const gltfjson::format::Root& root,
                   const gltfjson::format::Bin& bin,
                   std::optional<uint32_t> texture,
                   libvrm::gltf::ColorSpace colorspace);

void
RenderLine(const RenderingEnv& camera, std::span<const cuber::LineVertex> data);

}
