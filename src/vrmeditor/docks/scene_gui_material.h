#pragma once
#include <memory>
#include <vrm/material.h>

void
ShowMaterialPbr(const std::shared_ptr<libvrm::gltf::Material>& material);
void
ShowMaterialUnlit(const std::shared_ptr<libvrm::gltf::Material>& material);
void
ShowMaterialMToon0(const std::shared_ptr<libvrm::gltf::Material>& material);
void
ShowMaterialMToon1(const std::shared_ptr<libvrm::gltf::Material>& material);
