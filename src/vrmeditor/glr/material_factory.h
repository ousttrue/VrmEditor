#pragma once
#include "error_check.h"
#include "shader_factory.h"
#include "shader_source.h"
#include <expected>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/shader_type_name.h>
#include <grapho/gl3/texture.h>
#include <list>
#include <optional>
#include <unordered_map>
#include <vector>

namespace glr {

using UniformVar =
  std::variant<IntVar, FloatVar, Vec3Var, Vec4Var, Mat3Var, Mat4Var>;

struct MaterialFactory
{
  ShaderTypes Type;
  ShaderFactory VS;
  ShaderFactory FS;
  std::expected<std::shared_ptr<grapho::gl3::ShaderProgram>, std::string>
    Compiled = std::unexpected{ "init" };
  std::list<grapho::gl3::TextureSlot> Textures;

  std::unordered_map<std::string, UniformVar> UniformVarMap;
  std::vector<std::optional<UniformVar>> UniformVars;

  void Activate(const std::shared_ptr<ShaderSourceManager>& shaderSource,
                const WorldInfo& world,
                const LocalInfo& local,
                const gltfjson::tree::NodePtr& material)
  {
    if (!Compiled) {
      // execute mcaro
      for (auto& g : VS.MacroGroups) {
        for (auto& m : g.second) {
          std::visit([&world, &local, &material](
                       auto& var) { var.Update(world, local, material); },
                     m.Value);
        }
      }
      for (auto& g : FS.MacroGroups) {
        for (auto& m : g.second) {
          std::visit([&world, &local, &material](
                       auto& var) { var.Update(world, local, material); },
                     m.Value);
        }
      }
      auto vs = VS.Expand(Type, shaderSource);
      auto fs = FS.Expand(Type, shaderSource);
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
    if (Compiled) {
      auto shader = *Compiled;
      shader->Use();
      for (auto& texture : Textures) {
        texture.Activate();
      }
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
    }
  }
};

using MaterialFactoryFunc = std::function<std::shared_ptr<MaterialFactory>(
  const gltfjson::typing::Root& root,
  const gltfjson::typing::Bin& bin,
  std::optional<uint32_t> materialId)>;
}
