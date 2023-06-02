#pragma once
#include <gltfjson.h>

namespace glr {

struct MaterialFactory;
std::shared_ptr<MaterialFactory>
MaterialFactory_MToon(const gltfjson::typing::Root& root,
                      const gltfjson::typing::Bin& bin,
                      std::optional<uint32_t> materialId);

}
