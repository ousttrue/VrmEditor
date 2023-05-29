#pragma once
#include "app.h"
#include "gl3renderer.h"
#include "shader_source.h"
#include <gltfjson.h>
#include <grapho/gl3/material.h>
#include <grapho/gl3/pbr.h>

namespace glr {

inline std::expected<std::shared_ptr<grapho::gl3::Material>, std::string>
MaterialFactory_Pbr_LearnOpenGL(
  const std::shared_ptr<ShaderSourceManager>& shaderSource,
  const gltfjson::typing::Root& root,
  const gltfjson::typing::Bin& bin,
  std::optional<uint32_t> materialId)
{
  if (!materialId) {
    return {};
  }
  auto src = root.Materials[*materialId];
  std::shared_ptr<grapho::gl3::Texture> albedo;
  std::shared_ptr<grapho::gl3::Texture> metallic;
  std::shared_ptr<grapho::gl3::Texture> roughness;
  if (auto pbr = src.PbrMetallicRoughness()) {
    if (auto baseColorTexture = pbr->BaseColorTexture()) {
      albedo = GetOrCreateTexture(
        root, bin, baseColorTexture->Index(), ColorSpace::sRGB);
    }
    if (auto metallicRoughnessTexture = pbr->MetallicRoughnessTexture()) {
      metallic = GetOrCreateTexture(
        root, bin, metallicRoughnessTexture->Index(), ColorSpace::Linear);
      roughness = GetOrCreateTexture(
        root, bin, metallicRoughnessTexture->Index(), ColorSpace::Linear);
    }
  }
  std::shared_ptr<grapho::gl3::Texture> normal;
  if (auto normalTexture = src.NormalTexture()) {
    normal =
      GetOrCreateTexture(root, bin, normalTexture->Index(), ColorSpace::Linear);
  }
  std::shared_ptr<grapho::gl3::Texture> ao;
  if (auto occlusionTexture = src.OcclusionTexture()) {
    ao = GetOrCreateTexture(
      root, bin, occlusionTexture->Index(), ColorSpace::Linear);
  }

  std::vector<std::u8string_view> vs;
  std::vector<std::u8string_view> fs;
  vs.push_back(u8"#version 450\n");
  fs.push_back(u8"#version 450\n");
  if (albedo) {
    fs.push_back(u8"#define HAS_ALBEDO_TEXTURE\n");
  }
  if (metallic) {
    fs.push_back(u8"#define HAS_METALLIC_TEXTURE\n");
  }
  if (roughness) {
    fs.push_back(u8"#define HAS_ROUGHNESS_TEXTURE\n");
  }
  if (ao) {
    fs.push_back(u8"#define HAS_AO_TEXTURE\n");
  }
  if (normal) {
    fs.push_back(u8"#define HAS_NORMAL_TEXTURE\n");
  }

  auto vs_src = shaderSource->Get("pbr.vert");
  shaderSource->RegisterShaderType(vs_src, ShaderTypes::Pbr);
  vs.push_back(vs_src->Source);

  auto fs_src = shaderSource->Get("pbr.frag");
  shaderSource->RegisterShaderType(fs_src, ShaderTypes::Pbr);
  fs.push_back(fs_src->Source);

  return grapho::gl3::CreatePbrMaterial(
    albedo, normal, metallic, roughness, ao, vs, fs);
}

}
