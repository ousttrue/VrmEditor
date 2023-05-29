#pragma once
#include "shader_source.h"
#include <gltfjson.h>
#include <grapho/gl3/material.h>

namespace glr {

inline std::expected<std::shared_ptr<grapho::gl3::Material>, std::string>
MaterialFactory_Unlit(const std::shared_ptr<ShaderSourceManager>& shaderSource,
                      const gltfjson::typing::Root& root,
                      const gltfjson::typing::Bin& bin,
                      std::optional<uint32_t> id)
{
  if (!id) {
    return std::unexpected{ "no material id" };
  }

  std::vector<std::u8string_view> vs;
  std::vector<std::u8string_view> fs;
  vs.push_back(u8"#version 450\n");
  fs.push_back(u8"#version 450\n");
  if (gltfjson::typing::GetAlphaMode(root, id) ==
      gltfjson::format::AlphaModes::Mask) {
    fs.push_back(u8"#define MODE_MASK\n");
  }

  auto src = root.Materials[*id];

  auto vs_src = shaderSource->Get("unlit.vert");
  shaderSource->RegisterShaderType(vs_src, ShaderTypes::Unlit);
  auto fs_src = shaderSource->Get("unlit.frag");
  shaderSource->RegisterShaderType(fs_src, ShaderTypes::Unlit);
  vs.push_back(vs_src->Source);
  fs.push_back(fs_src->Source);
  if (auto shader = grapho::gl3::ShaderProgram::Create(vs, fs)) {
    auto material = std::make_shared<grapho::gl3::Material>();
    material->Shader = *shader;
    if (auto pbr = src.PbrMetallicRoughness()) {
      if (auto baseColorTexture = pbr->BaseColorTexture()) {
        if (auto texture = GetOrCreateTexture(
              root, bin, baseColorTexture->Index(), ColorSpace::sRGB)) {
          material->Textures.push_back({ 0, texture });
        }
      }
    }
    return material;
  } else {
    App::Instance().Log(LogLevel::Error) << shader.error();
    return std::unexpected{ shader.error() };
  }
}

}
