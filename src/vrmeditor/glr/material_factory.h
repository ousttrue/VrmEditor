#pragma once
#include "shader_source.h"
#include <DirectXMath.h>
#include <expected>
#include <functional>
#include <gltfjson.h>
#include <grapho/gl3/material.h>
#include <optional>
#include <sstream>
#include <variant>

namespace glr {

using UpdateShaderFunc =
  std::function<void(const std::shared_ptr<grapho::gl3::ShaderProgram>& shader,
                     const grapho::gl3::Material::EnvVars&,
                     const grapho::gl3::Material::DrawVars&,
                     const DirectX::XMFLOAT4X4& shadow)>;

struct ShaderDefinition
{
  std::u8string Name;
  std::variant<std::monostate, bool, int, float, std::u8string> Value;

  std::u8string Str() const
  {
    struct Visitor
    {
      std::string_view Name;
      std::stringstream m_ss;
      std::u8string operator()(std::monostate)
      {
        m_ss << "#define " << Name;
        auto str = m_ss.str();
        return { (const char8_t*)str.data(), str.size() };
      }
      std::u8string operator()(bool b)
      {
        m_ss << "#define " << Name;
        if (b) {
          m_ss << "#define " << Name << " true";
        } else {
          m_ss << "#define " << Name << " false";
        }
        auto str = m_ss.str();
        return { (const char8_t*)str.data(), str.size() };
      }
      std::u8string operator()(int value)
      {
        m_ss << "#define " << Name << " " << value;
        auto str = m_ss.str();
        return { (const char8_t*)str.data(), str.size() };
      }
      std::u8string operator()(float value)
      {
        m_ss << "#define " << Name << " " << value;
        auto str = m_ss.str();
        return { (const char8_t*)str.data(), str.size() };
      }
      std::u8string operator()(const std::u8string& value)
      {
        m_ss << "#define " << Name << " " << gltfjson::tree::from_u8(value);
        auto str = m_ss.str();
        return { (const char8_t*)str.data(), str.size() };
      }
    };

    return std::visit(Visitor{ gltfjson::tree::from_u8(Name) }, Value);
  }
};

struct ShaderFactory
{
  std::string SourceName;
  std::u8string Version;
  std::vector<ShaderDefinition> Macros;
  std::u8string SourceExpanded;
  std::u8string FullSource;

  std::u8string Expand(ShaderTypes type,
                       const std::shared_ptr<ShaderSourceManager>& shaderSource)
  {
    FullSource.clear();

    auto src = shaderSource->Get(SourceName);
    shaderSource->RegisterShaderType(src, type);
    SourceExpanded = src->Source;

    FullSource += Version;
    FullSource.push_back('\n');

    for (auto m : Macros) {
      FullSource += m.Str();
      FullSource.push_back('\n');
    }

    FullSource += SourceExpanded;
    FullSource.push_back('\n');

    return FullSource;
  }
};

struct MaterialFactory
{
  ShaderTypes Type;
  ShaderFactory VS;
  ShaderFactory FS;
  std::expected<std::shared_ptr<grapho::gl3::ShaderProgram>, std::string>
    Compiled;
  std::shared_ptr<grapho::gl3::Material> Material;
  UpdateShaderFunc Updater;
  std::list<grapho::gl3::TextureSlot> Textures;

  void Activate(const std::shared_ptr<ShaderSourceManager>& shaderSource,
                const grapho::gl3::Material::EnvVars& env,
                const grapho::gl3::Material::DrawVars& model,
                const DirectX::XMFLOAT4X4& shadow = {})
  {
    if (!Material) {
      Material = std::make_shared<grapho::gl3::Material>();

      auto vs = VS.Expand(Type, shaderSource);
      auto fs = FS.Expand(Type, shaderSource);
      Compiled = grapho::gl3::ShaderProgram::Create(vs, fs);
      if (Compiled) {
        Material->Shader = *Compiled;
        Material->Textures = Textures;
      }
    }
    if (Material && Material->Shader) {
      if (Updater) {
        Material->Shader->Use();
        Updater(Material->Shader, env, model, shadow);
      }
      Material->Activate();
    }
  }
};

using MaterialFactoryFunc = std::function<std::shared_ptr<MaterialFactory>(
  const gltfjson::typing::Root& root,
  const gltfjson::typing::Bin& bin,
  std::optional<uint32_t> materialId)>;
}
