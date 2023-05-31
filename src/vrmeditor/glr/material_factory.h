#pragma once
#include "app.h"
#include "error_check.h"
#include "rendering_env.h"
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

struct WorldInfo
{
  const RenderingEnv& m_env;

  DirectX::XMFLOAT4X4 ProjectionMatrix() const
  {
    return m_env.ProjectionMatrix;
  }
  DirectX::XMFLOAT4X4 ViewMatrix() const { return m_env.ViewMatrix; }
  DirectX::XMFLOAT4X4 ViewProjectionMatrix() const
  {
    DirectX::XMFLOAT4X4 m;
    DirectX::XMStoreFloat4x4(
      &m,
      DirectX::XMLoadFloat4x4(&m_env.ViewMatrix) *
        DirectX::XMLoadFloat4x4(&m_env.ProjectionMatrix));
    return m;
  }
  DirectX::XMFLOAT4X4 ShadowMatrix() const { return m_env.ShadowMatrix; }
  DirectX::XMFLOAT3 CameraPosition() const { return m_env.CameraPosition; }
};
struct LocalInfo
{
  const grapho::gl3::Material::LocalVars& m_local;

  DirectX::XMFLOAT4X4 ModelMatrix() const { return m_local.model; }
  DirectX::XMFLOAT4X4 NormalMatrix4() const { return m_local.normalMatrix; }
  DirectX::XMFLOAT3X3 NormalMatrix3() const { return m_local.normalMatrix3(); }
  DirectX::XMFLOAT4 ColorRGBA() const { return m_local.color; }
  DirectX::XMFLOAT3 EmissiveRGB() const { return m_local.emissiveColor; }
  DirectX::XMFLOAT3X3 UvTransformMatrix() const { return m_local.uvTransform(); }
};

using UpdateShaderFunc =
  std::function<void(const std::shared_ptr<grapho::gl3::ShaderProgram>& shader,
                     const WorldInfo&,
                     const LocalInfo&)>;

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

struct ShaderEnum
{
  struct KeyValue
  {
    std::u8string Name;
    int Value;

    std::u8string Str() const
    {
      std::stringstream ss;
      ss << "#define " << gltfjson::tree::from_u8(Name) << " " << Value;
      auto str = ss.str();
      return { (const char8_t*)str.data(), str.size() };
    }
  };
  std::vector<KeyValue> Values;
  ShaderDefinition Selected;
};

struct ShaderFactory
{
  std::string SourceName;
  std::u8string Version;
  std::u8string Precision;
  std::vector<ShaderEnum> Enums;
  std::vector<std::u8string> Codes;
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

    if (Precision.size()) {
      FullSource += u8"precision ";
      FullSource += Precision;
      if (FullSource.back() != ';') {
        FullSource.push_back(';');
      }
      FullSource.push_back('\n');
    }

    for (auto& e : Enums) {
      for (auto& kv : e.Values) {
        FullSource += kv.Str();
        FullSource.push_back('\n');
      }
      FullSource += e.Selected.Str();
      FullSource.push_back('\n');
    }

    for (auto& m : Macros) {
      FullSource += m.Str();
      FullSource.push_back('\n');
    }

    for (auto& c : Codes) {
      FullSource += c;
      FullSource.push_back('\n');
    }

    FullSource += SourceExpanded;
    FullSource.push_back('\n');

    return FullSource;
  }
};

template<typename T>
using GetterFunc = std::function<T(const WorldInfo&, const LocalInfo&)>;

GetterFunc<int>
GetInt(int value)
{
  return [value](auto, auto) { return value; };
}
GetterFunc<float>
GetFloat(float value)
{
  return [value](auto, auto) { return value; };
}

struct UniformBind
{
  std::string Name;
  std::variant<GetterFunc<int>,
               GetterFunc<float>,
               GetterFunc<DirectX::XMFLOAT3>,
               GetterFunc<DirectX::XMFLOAT4>,
               GetterFunc<DirectX::XMFLOAT3X3>,
               GetterFunc<DirectX::XMFLOAT4X4>>
    Getter;
};

struct MaterialFactory
{
  ShaderTypes Type;
  ShaderFactory VS;
  ShaderFactory FS;
  std::expected<std::shared_ptr<grapho::gl3::ShaderProgram>, std::string>
    Compiled;
  std::shared_ptr<grapho::gl3::Material> Material;
  std::list<grapho::gl3::TextureSlot> Textures;
  // UpdateShaderFunc Updater;
  std::list<UniformBind> UniformBinds;

  void Activate(const std::shared_ptr<ShaderSourceManager>& shaderSource,
                const WorldInfo& world,
                const LocalInfo& local)
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
    ERROR_CHECK;
    if (Material && Material->Shader) {
      // Material->Shader->Use();
      Material->Activate();
      GL_ErrorClear("UBO");
      for (auto& bind : UniformBinds) {
        // GL_ErrorCheck("before %s", bind.Name.c_str());
        std::visit(
          [shader = Material->Shader, &bind, &world, &local](
            const auto& getter) {
            //
            ERROR_CHECK;
            shader->SetUniform(bind.Name, getter(world, local));
            ERROR_CHECK;
          },
          bind.Getter);
        // GL_ErrorCheck("after %s", bind.Name.c_str());
      }
    }
  }
};

using MaterialFactoryFunc = std::function<std::shared_ptr<MaterialFactory>(
  const gltfjson::typing::Root& root,
  const gltfjson::typing::Bin& bin,
  std::optional<uint32_t> materialId)>;
}
