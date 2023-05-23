#pragma once
#include <gltfjson.h>
#include <string>
#include <vrm/gltf.h>

void
ShowGui(std::list<gltfjson::format::Extension>& extensions);
void
ShowGui(std::list<gltfjson::format::Extra>& extras);

void
ShowGui(gltfjson::format::Asset& asset);

void
ShowText(const std::u8string& text);

// buffer/bufferView/accessor
void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Buffer& buffer);
void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::BufferView& bufferView);
void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Accessor& accessor);
// image/sampler/texture/material/mesh
void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Image& image);
void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Sampler& sampler);
void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Texture& texture);
void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Material& material);
void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Mesh& mesh);
void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::MeshPrimitive& prim);
void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::MeshPrimitiveMorphTarget& target);
// skin/node/scene/animation
void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Skin& skin);
void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Node& node);
void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Scene& scene);
void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Animation& animation);
