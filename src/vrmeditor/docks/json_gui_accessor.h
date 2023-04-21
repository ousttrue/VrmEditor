#pragma once
#include "json_gui.h"

ShowGui
JsonGuiAccessor(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                std::string_view jsonpath);

ShowGui
JsonGuiAccessorList(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                    std::string_view jsonpath);
