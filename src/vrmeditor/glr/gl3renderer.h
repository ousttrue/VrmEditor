#pragma once
#include "docks/gui.h"
#include "renderpass.h"
#include <cuber/mesh.h>
#include <memory>

namespace libvrm {
namespace gltf {
struct Mesh;
class Image;
}
}
namespace grapho {
namespace gl3 {
struct Texture;
}
}

namespace runtimescene {
struct RuntimeMesh;
}

namespace glr {
struct RenderingEnv;

void
Render(RenderPass pass,
       const RenderingEnv& camera,
       const std::shared_ptr<libvrm::gltf::Mesh>& mesh,
       const runtimescene::RuntimeMesh& instance,
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
GetOrCreate(const std::shared_ptr<libvrm::gltf::Image>& image);

void
RenderLine(const RenderingEnv& camera, std::span<const cuber::LineVertex> data);

}
