#pragma once
#include <GL/glew.h>

#include "glr/gl3renderer.h"
#include "json_gui.h"
#include "json_gui_table.h"
#include <grapho/gl3/texture.h>
#include <imgui.h>
#include <vrm/material.h>

inline ShowGui
JsonGuiImageList(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                    std::string_view jsonpath)
{
  std::vector<std::weak_ptr<grapho::gl3::Texture>> textures;
  for (auto& image : scene->m_images) {
    auto texture = glr::GetOrCreate(image);
    textures.push_back(texture);
  }
  return [&images = scene->m_images, textures]() {
    //
    std::array<const char*, 5> cols = {
      "index", "name", "image", "width", "height",
    };
    if (JsonGuiTable("##images", cols)) {

      for (int i = 0; i < textures.size(); ++i) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", i);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", images[i]->Name.c_str());
        ImGui::TableSetColumnIndex(2);
        if (auto texture = textures[i].lock()) {
          ImGui::Image((ImTextureID)(uint64_t)texture->texture_, { 150, 150 });
        }
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%d", images[i]->Width());
        ImGui::TableSetColumnIndex(4);
        ImGui::Text("%d", images[i]->Height());
      }

      ImGui::EndTable();
    }
  };
}
