#include "tag.h"
#include <gltfjson/gltf_typing_vrm0.h>
#include <gltfjson/gltf_typing_vrm1.h>
#include <imgui.h>

ShowTagFunc
MaterialTag(const gltfjson::Root& root,
            const gltfjson::Bin& bin,
            const gltfjson::tree::NodePtr& item)
{

  for (int i = 0; i < root.Materials.size(); ++i) {
    auto m = root.Materials[i];
    if (m.m_json == item) {
      if (m.GetExtension<gltfjson::vrm1::MToon>()) {
        return []() { ImGui::SmallButton("mtoon1"); };
      } else if (gltfjson::vrm0::GetVrmMaterial(root, i)) {
        return []() { ImGui::SmallButton("mtoon0"); };
      } else if (m.GetExtension<gltfjson::KHR_materials_unlit>()) {
        return []() { ImGui::SmallButton("unlit"); };
      } else {
        return []() { ImGui::SmallButton("pbr"); };
      }
    }
  }

  return {};
}
