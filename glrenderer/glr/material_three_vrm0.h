#include "material.h"

namespace glr {

std::shared_ptr<Material>
MaterialFactory_MToon0(const gltfjson::Root& root,
                       const gltfjson::Bin& bin,
                       uint32_t materialId,
                       const gltfjson::tree::NodePtr& mtoon0);

}
