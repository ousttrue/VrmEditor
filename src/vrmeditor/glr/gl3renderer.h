#pragma once
#include <memory>

struct ViewProjection;
namespace libvrm {
namespace gltf {
struct Mesh;
struct MeshInstance;
} // namespace gltf
}

namespace glr {

void
Render(const ViewProjection& camera,
       const std::shared_ptr<libvrm::gltf::Mesh>& mesh,
       const libvrm::gltf::MeshInstance& instance,
       const float m[16]);

// clear current render target
void
ClearRendertarget(const ViewProjection& camera);

// release all resource
void
Shutdown();

// resource viewer
void
ShowGui();

}
