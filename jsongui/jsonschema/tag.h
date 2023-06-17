#pragma once
#include "json_prop.h"

ShowTagFunc
AccessorTag(const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            const gltfjson::tree::NodePtr& item);

ShowTagFunc
ImageTag(const gltfjson::Root& root,
         const gltfjson::Bin& bin,
         const gltfjson::tree::NodePtr& item);

ShowTagFunc
MaterialTag(const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            const gltfjson::tree::NodePtr& item);

ShowTagFunc
MeshTag(const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            const gltfjson::tree::NodePtr& item);

ShowTagFunc
NodeTag(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        const gltfjson::tree::NodePtr& item);
