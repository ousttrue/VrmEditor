#pragma once
#include <vrm/animation.h>

namespace runtimescene {

void
AnimationUpdate(const libvrm::gltf::Animation& animation,
                libvrm::Time time,
                std::span<std::shared_ptr<libvrm::gltf::Node>> nodes,
                bool repeat = false);

}
