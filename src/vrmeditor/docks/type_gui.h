#pragma once
#include <gltfjson.h>
#include <string>
#include <vrm/gltf.h>

// void
// ShowGui(std::list<gltfjson::annotation::Extension>& extensions);
// void
// ShowGui(std::list<gltfjson::annotation::Extra>& extras);

void
ShowGui(gltfjson::annotation::Asset& asset);

void
ShowText(const std::u8string& text);

// buffer/bufferView/accessor
void
ShowGui(const gltfjson::annotation::Root& root,
        const gltfjson::annotation::Bin& bin,
        gltfjson::annotation::Buffer& buffer);
void
ShowGui(const gltfjson::annotation::Root& root,
        const gltfjson::annotation::Bin& bin,
        gltfjson::annotation::BufferView& bufferView);
void
ShowGui(const gltfjson::annotation::Root& root,
        const gltfjson::annotation::Bin& bin,
        gltfjson::annotation::Accessor& accessor);
// image/sampler/texture/material/mesh
void
ShowGui(const gltfjson::annotation::Root& root,
        const gltfjson::annotation::Bin& bin,
        gltfjson::annotation::Image& image);
void
ShowGui(const gltfjson::annotation::Root& root,
        const gltfjson::annotation::Bin& bin,
        gltfjson::annotation::Sampler& sampler);
void
ShowGui(const gltfjson::annotation::Root& root,
        const gltfjson::annotation::Bin& bin,
        gltfjson::annotation::Texture& texture);
void
ShowGui(const gltfjson::annotation::Root& root,
        const gltfjson::annotation::Bin& bin,
        gltfjson::annotation::Material& material);
void
ShowGui(const gltfjson::annotation::Root& root,
        const gltfjson::annotation::Bin& bin,
        gltfjson::annotation::Mesh& mesh);
void
ShowGui(const gltfjson::annotation::Root& root,
        const gltfjson::annotation::Bin& bin,
        gltfjson::annotation::MeshPrimitive& prim);
void
ShowGui(const gltfjson::annotation::Root& root,
        const gltfjson::annotation::Bin& bin,
        gltfjson::annotation::MeshPrimitiveMorphTarget& target);
// skin/node/scene/animation
void
ShowGui(const gltfjson::annotation::Root& root,
        const gltfjson::annotation::Bin& bin,
        gltfjson::annotation::Skin& skin);
void
ShowGui(const gltfjson::annotation::Root& root,
        const gltfjson::annotation::Bin& bin,
        gltfjson::annotation::Node& node);
void
ShowGui(const gltfjson::annotation::Root& root,
        const gltfjson::annotation::Bin& bin,
        gltfjson::annotation::Scene& scene);
void
ShowGui(const gltfjson::annotation::Root& root,
        const gltfjson::annotation::Bin& bin,
        gltfjson::annotation::Animation& animation);
