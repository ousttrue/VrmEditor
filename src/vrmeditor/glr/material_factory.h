#pragma once
#include "app.h"
#include "error_check.h"
#include "rendering_env.h"
#include "shader_source.h"
#include <DirectXMath.h>
#include <expected>
#include <functional>
#include <gltfjson.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/shader_type_name.h>
#include <grapho/gl3/texture.h>
#include <grapho/vars.h>
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
  const grapho::LocalVars& m_local;

  DirectX::XMFLOAT4X4 ModelMatrix() const { return m_local.model; }
  DirectX::XMFLOAT4X4 NormalMatrix4() const { return m_local.normalMatrix; }
  DirectX::XMFLOAT3X3 NormalMatrix3() const { return m_local.normalMatrix3(); }
  DirectX::XMFLOAT4 ColorRGBA() const { return m_local.color; }
  DirectX::XMFLOAT3 EmissiveRGB() const { return m_local.emissiveColor; }
  DirectX::XMFLOAT3X3 UvTransformMatrix() const
  {
    return m_local.uvTransform();
  }
};

using UpdateShaderFunc =
  std::function<void(const std::shared_ptr<grapho::gl3::ShaderProgram>& shader,
                     const WorldInfo&,
                     const LocalInfo&)>;

struct ShaderDefinition
{
  std::u8string Name;
  std::variant<std::monostate, bool, int, float, std::u8string> Value;
  bool Checked = true;
};

struct ShaderDefinitionToStorVisitor
{
  ShaderDefinition& Def;

  std::string_view Name() const
  {
    return { (const char*)Def.Name.data(), Def.Name.size() };
  }

  std::stringstream m_ss;
  std::u8string operator()(std::monostate)
  {
    std::string str;
    if (Def.Checked) {
      m_ss << "#define " << Name();
      str = m_ss.str();
    }
    return { (const char8_t*)str.data(), str.size() };
  }
  std::u8string operator()(bool b)
  {
    if (b) {
      m_ss << "#define " << Name() << " true";
    } else {
      m_ss << "#define " << Name() << " false";
    }
    auto str = m_ss.str();
    return { (const char8_t*)str.data(), str.size() };
  }
  std::u8string operator()(int value)
  {
    m_ss << "#define " << Name() << " " << value;
    auto str = m_ss.str();
    return { (const char8_t*)str.data(), str.size() };
  }
  std::u8string operator()(float value)
  {
    m_ss << "#define " << Name() << " " << value;
    auto str = m_ss.str();
    return { (const char8_t*)str.data(), str.size() };
  }
  std::u8string operator()(const std::u8string& value)
  {
    m_ss << "#define " << Name() << " " << gltfjson::tree::from_u8(value);
    auto str = m_ss.str();
    return { (const char8_t*)str.data(), str.size() };
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
      FullSource +=
        std::visit(ShaderDefinitionToStorVisitor{ e.Selected }, e.Selected.Value);
      FullSource.push_back('\n');
    }

    for (auto& m : Macros) {
      FullSource += std::visit(ShaderDefinitionToStorVisitor{ m }, m.Value);
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
using GetterFunc = std::function<T(const WorldInfo&,
                                   const LocalInfo&,
                                   const gltfjson::tree::NodePtr& material)>;

GetterFunc<int>
GetInt(int value)
{
  return [value](auto, auto, auto) { return value; };
}
GetterFunc<float>
GetFloat(float value)
{
  return [value](auto, auto, auto) { return value; };
}

using IntGetter = GetterFunc<int>;
using FloatGetter = GetterFunc<float>;
using Vec3Getter = GetterFunc<DirectX::XMFLOAT3>;
using Vec4Getter = GetterFunc<DirectX::XMFLOAT4>;
using Mat3Getter = GetterFunc<DirectX::XMFLOAT3X3>;
using Mat4Getter = GetterFunc<DirectX::XMFLOAT4X4>;

using GetterVariant = std::variant<IntGetter,
                                   FloatGetter,
                                   Vec3Getter,
                                   Vec4Getter,
                                   Mat3Getter,
                                   Mat4Getter>;

using BindFunc = std::function<void(const WorldInfo&,

                                    const LocalInfo&,
                                    const gltfjson::tree::NodePtr& material)>;

struct MaterialFactory
{
  ShaderTypes Type;
  ShaderFactory VS;
  ShaderFactory FS;
  std::expected<std::shared_ptr<grapho::gl3::ShaderProgram>, std::string>
    Compiled = std::unexpected{ "init" };
  std::list<grapho::gl3::TextureSlot> Textures;

  std::unordered_map<std::string, GetterVariant> UniformGetterMap;
  std::vector<std::optional<GetterVariant>> UniformGetters;

  void Activate(const std::shared_ptr<ShaderSourceManager>& shaderSource,
                const WorldInfo& world,
                const LocalInfo& local,
                const gltfjson::tree::NodePtr& material)
  {
    if (!Compiled) {
      auto vs = VS.Expand(Type, shaderSource);
      auto fs = FS.Expand(Type, shaderSource);
      Compiled = grapho::gl3::ShaderProgram::Create(vs, fs);

      // match binding
      if (Compiled) {
        auto shader = *Compiled;
        UniformGetters.clear();
        for (auto& u : shader->Uniforms) {
          auto found = UniformGetterMap.find(u.Name);
          if (found != UniformGetterMap.end()) {
            UniformGetters.push_back(found->second);
          } else {
            UniformGetters.push_back({});
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
          auto& g = UniformGetters[i];
          if (g) {
#ifndef NDEBUG
            std::string type_name = grapho::gl3::ShaderTypeName(u.Type);
            auto debug = shader->Uniform(u.Name);
            assert(debug->Location == u.Location);
#endif
            GL_ErrorCheck("before");
            std::visit(
              [&u, &world, &local, &material](const auto& getter) {
                //
                u.Set(getter(world, local, material));
              },
              *g);
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
