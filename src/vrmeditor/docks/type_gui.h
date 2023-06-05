#pragma once
#include <gltfjson.h>
#include <string>

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Asset asset);

bool
ShowText(const std::u8string& text);

// buffer/bufferView/accessor
bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Buffer buffer);
bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::BufferView bufferView);
bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Accessor accessor);
// image/sampler/texture/material/mesh
bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Image image);
bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Sampler sampler);
bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Texture texture);
bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Material material);
bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Mesh mesh);
bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::MeshPrimitive prim);
bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::MeshPrimitiveMorphTarget target);
// skin/node/scene/animation
bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Skin skin);
bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Node node);
bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Scene scene);
bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Animation animation);
