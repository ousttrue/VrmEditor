#include <GL/glew.h>

#include "material.h"
#include <grapho/gl3/cubemap.h>
#include <grapho/gl3/glsl_type_name.h>

namespace glr {

void
Material::Activate(const std::shared_ptr<ShaderSourceManager>& shaderSource,
                   const WorldInfo& world,
                   const LocalInfo& local,
                   const Gltf& gltf)
{
  if (!Compiled) {
    auto error = Compiled.error();
    if (error.empty()) {
      // execute mcaro
      VS.Update(world, local, gltf);
      auto vs = VS.Expand(shaderSource);
      FS.Update(world, local, gltf);
      auto fs = FS.Expand(shaderSource);
      GS.Update(world, local, gltf);
      auto gs = GS.Expand(shaderSource);
      Compiled = grapho::gl3::ShaderProgram::Create(vs, fs, gs);

      // match binding
      if (Compiled) {
        auto shader = *Compiled;
        UniformVars.clear();
        for (auto& u : shader->Uniforms) {
          auto found = UniformVarMap.find(u.Name);
          if (found != UniformVarMap.end()) {
            std::visit([&world, &local, &gltf](
                         auto& var) { var.Update(world, local, gltf); },
                       found->second);
            UniformVars.push_back(found->second);
          } else {
            UniformVars.push_back({});
          }
        }
      } else {
        auto error = Compiled.error();
        auto a = 0;
      }
    }
  }
  if (Compiled) {
    if (UpdateState) {
      UpdateState(world, local, gltf);
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
          assert(!grapho::gl3::TryGetError());
          std::visit(
            [&u, &world, &local, &gltf](const auto& var) {
              //
              u.Set(var.Update(world, local, gltf));
            },
            *v);
          assert(!grapho::gl3::TryGetError());
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
