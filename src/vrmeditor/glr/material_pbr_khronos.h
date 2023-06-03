#pragma once
#include <gltfjson.h>

namespace glr {

struct Material;
std::shared_ptr<Material>
MaterialFactory_Pbr_Khronos_GLTF(const gltfjson::typing::Root& root,
                            const gltfjson::typing::Bin& bin,
                            std::optional<uint32_t> materialId);

} // namespace
