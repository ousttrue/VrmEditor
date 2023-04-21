#pragma once
#include "json_gui.h"

ShowGui
JsonGuiMaterialList(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                    std::string_view jsonpath);
