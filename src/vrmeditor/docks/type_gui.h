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
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Buffer& buffer);
void
ShowGui(const gltfjson::format::Root& root,
        gltfjson::format::BufferView& bufferView);
void
ShowGui(const gltfjson::format::Root& root,
        gltfjson::format::Accessor& accessor);
// image/sampler/texture/material/mesh
void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Image& image);
void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Sampler& sampler);
void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Texture& texture);
void
ShowGui(const gltfjson::format::Root& root,
        gltfjson::format::Material& material);
void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Mesh& mesh);
// skin/node/scene/animation
void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Skin& skin);
void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Node& node);
void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Scene& scene);
void
ShowGui(const gltfjson::format::Root& root,
        gltfjson::format::Animation& animation);
