#pragma once
#include "animation.h"

namespace runtimescene {
struct RuntimeScene;

void
AnimationUpdate(const Animation& animation,
                libvrm::Time time,
                std::span<std::shared_ptr<libvrm::gltf::Node>> nodes,
                const std::shared_ptr<RuntimeScene>& runtime,
                bool repeat = false);

}
