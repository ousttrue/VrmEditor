#pragma once
#include "error_check.h"
#include "gl3renderer.h"
#include "shader_factory.h"
#include "shader_source.h"
#include <expected>
#include <grapho/gl3/cubemap.h>
#include <grapho/gl3/glsl_type_name.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/texture.h>
#include <list>
#include <optional>
#include <unordered_map>
#include <vector>

namespace glr {

using UniformVar =
  std::variant<IntVar, FloatVar, Vec3Var, Vec4Var, Mat3Var, Mat4Var, RgbVar, RgbaVar>;

struct EnvTextureBind
{
  uint32_t Slot;
  EnvTextureTypes Type;
};
struct EnvCubemapBind
{
  uint32_t Slot;
  EnvCubemapTypes Type;
};

struct Material
{
  std::string Name;
  ShaderFactory VS;
  ShaderFactory FS;
  std::expected<std::shared_ptr<grapho::gl3::ShaderProgram>, std::string>
    Compiled = std::unexpected{ "" };
  std::list<grapho::gl3::TextureSlot> Textures;
  std::list<EnvCubemapBind> EnvCubemaps;
  std::list<EnvTextureBind> EnvTextures;
  std::unordered_map<std::string, UniformVar> UniformVarMap;
  std::vector<std::optional<UniformVar>> UniformVars;

  std::function<void(const WorldInfo& world,
                     const LocalInfo& local,
                     const gltfjson::tree::NodePtr& material)>
    UpdateState;

  void Activate(const std::shared_ptr<ShaderSourceManager>& shaderSource,
                const WorldInfo& world,
                const LocalInfo& local,
                const gltfjson::tree::NodePtr& material)
  {
    if (!Compiled) {
      auto error = Compiled.error();
      if (error.empty()) {
        // execute mcaro
        VS.Update(world, local, material);
        auto vs = VS.Expand(shaderSource);
        FS.Update(world, local, material);
        auto fs = FS.Expand(shaderSource);
        Compiled = grapho::gl3::ShaderProgram::Create(vs, fs);

        // match binding
        if (Compiled) {
          auto shader = *Compiled;
          UniformVars.clear();
          for (auto& u : shader->Uniforms) {
            auto found = UniformVarMap.find(u.Name);
            if (found != UniformVarMap.end()) {
              std::visit([&world, &local, &material](
                           auto& var) { var.Update(world, local, material); },
                         found->second);
              UniformVars.push_back(found->second);
            } else {
              UniformVars.push_back({});
            }
          }
        }
      }
    }
    if (Compiled) {
      if (UpdateState) {
        UpdateState(world, local, material);
      }

      auto shader = *Compiled;
      shader->Use();

      for (size_t i = 0; i < shader->Uniforms.size(); ++i) {
        auto& u = shader->Uniforms[i];
        if (u.Location != -1) {
          auto& v = UniformVars[i];
          if (v) {
#ifndef NDEBUG
            std::string type_name = grapho::gl3::ShaderTypeName(u.Type);
            auto debug = shader->Uniform(u.Name);
            assert(debug->Location == u.Location);
#endif
            GL_ErrorCheck("before");
            std::visit(
              [&u, &world, &local, &material](const auto& var) {
                //
                u.Set(var.Update(world, local, material));
              },
              *v);
            GL_ErrorClear("after");
          }
        }
      }

      for (auto& texture : Textures) {
        texture.Activate();
      }
      for (auto& bind : EnvTextures) {
        if (auto texture = glr::GetEnvTexture(bind.Type)) {
          texture->Activate(bind.Slot);
        }
      }
      for (auto& bind : EnvCubemaps) {
        if (auto cubemap = glr::GetEnvCubemap(bind.Type)) {
          cubemap->Activate(bind.Slot);
        }
      }
    }
  }
};

using MaterialFactoryFunc = std::function<std::shared_ptr<Material>(
  const gltfjson::typing::Root& root,
  const gltfjson::typing::Bin& bin,
  std::optional<uint32_t> materialId)>;
}
