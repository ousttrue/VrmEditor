#pragma once
#include <GL/glew.h>

#include "glr/gl3renderer.h"
#include "json_gui.h"
#include "json_gui_table.h"
#include <grapho/gl3/texture.h>
#include <imgui.h>
#include <vrm/image.h>
#include <vrm/material.h>
#include <vrm/texture.h>

inline ShowGui
JsonGuiImageList(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                 std::string_view jsonpath)
{
  static TableColumn<std::shared_ptr<libvrm::gltf::Image>> ImageTable[]{

    { "index", [](auto i, const auto&) { ImGui::Text("%zu", i); } },
    { "name",
      [](auto, const auto& image) { ImGui::Text("%s", image->Name.c_str()); } },
    { "type",
      [](auto, const auto& image) {
        ImGui::TextUnformatted(image->Type().c_str());
      } },
    { "channels",
      [](auto, const auto& image) { ImGui::Text("%d", image->Channels()); } },
    { "width",
      [](auto, const auto& image) { ImGui::Text("%d", image->Width()); } },
    { "height",
      [](auto, const auto& image) { ImGui::Text("%d", image->Height()); } },
  };

  return TableToShowGui<std::shared_ptr<libvrm::gltf::Image>>(
    "##images", ImageTable, scene->m_images);
}

inline ShowGui
JsonGuiSamplerList(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                   std::string_view jsonpath)
{
  static TableColumn<std::shared_ptr<libvrm::gltf::TextureSampler>>
    SamplerTable[]{

      { "index", [](auto i, const auto&) { ImGui::Text("%zu", i); } },
      { "magFilter",
        [](auto, const auto& sampler) {
          ImGui::Text("%d", sampler->MagFilter);
        } },
      { "minFilter",
        [](auto, const auto& sampler) {
          ImGui::Text("%d", sampler->MinFilter);
        } },
      { "wrapS",
        [](auto, const auto& sampler) { ImGui::Text("%d", sampler->WrapS); } },
      { "wrapT",
        [](auto, const auto& sampler) { ImGui::Text("%d", sampler->WrapT); } },
    };

  return TableToShowGui<std::shared_ptr<libvrm::gltf::TextureSampler>>(
    "##samplers", SamplerTable, scene->m_samplers);
}

inline ShowGui
JsonGuiTextureList(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                   std::string_view jsonpath)
{
  static TableColumn<std::shared_ptr<libvrm::gltf::Texture>> TextureTable[]{

    { "index", [](auto i, const auto&) { ImGui::Text("%zu", i); } },
    { "image",
      [](auto, const auto& value) {
        if (auto texture = glr::GetOrCreate(value, value->ColorSpace)) {
          ImGui::Image((void*)(uint64_t)texture->texture_, { 150, 150 });
        }
      } },
  };

  return TableToShowGui<std::shared_ptr<libvrm::gltf::Texture>>(
    "##textures", TextureTable, scene->m_textures);
}
