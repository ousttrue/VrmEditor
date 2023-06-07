#include "material.h"

namespace glr {

std::shared_ptr<Material>
MaterialFactory_MToon1(const gltfjson::Root& root,
                       const gltfjson::Bin& bin,
                       uint32_t materialId,
                       const gltfjson::tree::NodePtr& mtoon1);

} // namespace
