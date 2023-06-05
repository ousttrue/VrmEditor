#pragma once
#include "animation.h"

namespace libvrm {

struct RuntimeScene;

void
AnimationUpdate(const Animation& animation,
                libvrm::Time time,
                std::span<std::shared_ptr<libvrm::Node>> nodes,
                const std::shared_ptr<RuntimeScene>& runtime,
                bool repeat = false);

}
