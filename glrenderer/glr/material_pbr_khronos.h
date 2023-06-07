#pragma once
#include <gltfjson.h>

namespace glr {

struct Material;
std::shared_ptr<Material>
MaterialFactory_Pbr_Khronos_GLTF(const gltfjson::Root& root,
                            const gltfjson::Bin& bin,
                            std::optional<uint32_t> materialId);

} // namespace
