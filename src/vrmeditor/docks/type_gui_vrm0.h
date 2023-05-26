#pragma once
#include <gltfjson/gltf_typing_vrm0.h>
#include <string>
#include <vrm/gltf.h>

// void
// ShowGui(std::list<gltfjson::annotation::Extension>& extensions);
// void
// ShowGui(std::list<gltfjson::annotation::Extra>& extras);

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::vrm0::VRM vrm);
