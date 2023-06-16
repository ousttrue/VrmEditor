#pragma once
#include "json_prop.h"

ShowTagFunc
MaterialTag(const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            const gltfjson::tree::NodePtr& item);

ShowTagFunc
NodeTag(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        const gltfjson::tree::NodePtr& item);
