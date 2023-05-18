#pragma once
#include <gltfjson.h>
#include <vrm/gltf.h>

////////////////////////////////////////////////////////////
// gltfjson
////////////////////////////////////////////////////////////
void
ShowGui(gltfjson::format::Asset& asset);
// buffer/bufferView/accessor
void
ShowGui(uint32_t index, gltfjson::format::Buffer& buffer);
void
ShowGui(uint32_t index, gltfjson::format::BufferView& bufferView);
void
ShowGui(uint32_t index, gltfjson::format::Accessor& accessor);
// image/sampler/texture/material/mesh
void
ShowGui(uint32_t index, gltfjson::format::Image& image);
void
ShowGui(uint32_t index, gltfjson::format::Sampler& sampler);
void
ShowGui(uint32_t index, gltfjson::format::Texture& texture);
void
ShowGui(uint32_t index, gltfjson::format::Material& material);
void
ShowGui(uint32_t index, gltfjson::format::Mesh& mesh);
// skin/node/scene/animation
void
ShowGui(uint32_t index, gltfjson::format::Skin& skin);
void
ShowGui(uint32_t index, gltfjson::format::Node& node);
void
ShowGui(uint32_t index, gltfjson::format::Scene& scene);
void
ShowGui(uint32_t index, gltfjson::format::Animation& animation);

////////////////////////////////////////////////////////////
// libvrm
////////////////////////////////////////////////////////////
inline void
ShowGui(uint32_t index,
        const std::shared_ptr<libvrm::gltf::GltfRoot>& root,
        const std::shared_ptr<gltfjson::format::Sampler>& sampler)
{
  ShowGui(index, *sampler);
}
void
ShowGui(uint32_t index,
        const std::shared_ptr<libvrm::gltf::GltfRoot>& root,
        const std::shared_ptr<libvrm::gltf::Texture>& texture);
void
ShowGui(uint32_t index,
        const std::shared_ptr<libvrm::gltf::GltfRoot>& root,
        const std::shared_ptr<libvrm::gltf::Material>& material);
