#include <GL/glew.h>

#include "material.h"
#include <grapho/gl3/cubemap.h>
#include <grapho/gl3/glsl_type_name.h>

namespace glr {

void
Material::Activate(const std::shared_ptr<ShaderSourceManager>& shaderSource,
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
}
