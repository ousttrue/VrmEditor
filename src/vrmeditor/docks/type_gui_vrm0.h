#pragma once
#include <gltfjson/gltf_typing_vrm0.h>
#include <string>
#include <vrm/gltf.h>

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::vrm0::VRM vrm);

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::vrm0::Meta meta);

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::vrm0::Humanoid humanoid);

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::vrm0::FirstPerson firstPerson);

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::vrm0::BlendShapeGroup blendShapeGroup);

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::vrm0::Spring spring);

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::vrm0::ColliderGroup colliderGroup);

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::vrm0::Material material);
