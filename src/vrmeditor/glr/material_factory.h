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

template<typename T>
struct Variable
{
  using GetterFunc = std::function<T(const WorldInfo&,
                                     const LocalInfo&,
                                     const gltfjson::tree::NodePtr& material)>;
  GetterFunc Getter;
  mutable T LastValue;
  T Update(const WorldInfo& w,
           const LocalInfo& l,
           const gltfjson::tree::NodePtr& material) const
  {
    if (GuiOverride) {
      LastValue = *GuiOverride;
    } else {
      LastValue = Getter(w, l, material);
    }
    return LastValue;
  }

  void Override(T value)
  {
    GuiOverride = value;
    LastValue = value;
  }

  std::optional<T> GuiOverride;
};

using OptVar = Variable<std::optional<std::monostate>>;
using BoolVar = Variable<bool>;
using IntVar = Variable<int>;
using FloatVar = Variable<float>;
using StringVar = Variable<std::string>;
using Vec3Var = Variable<DirectX::XMFLOAT3>;
using Vec4Var = Variable<DirectX::XMFLOAT4>;
using Mat3Var = Variable<DirectX::XMFLOAT3X3>;
using Mat4Var = Variable<DirectX::XMFLOAT4X4>;

auto
ConstInt(int value)
{
  return IntVar{ [value](auto, auto, auto) { return value; }, value };
}
auto
ConstFloat(float value)
{
  return FloatVar{ [value](auto, auto, auto) { return value; }, value };
}
auto
ConstBool(bool value)
{
  return BoolVar{ [value](auto, auto, auto) { return value; }, value };
}

using UpdateShaderFunc =
  std::function<void(const std::shared_ptr<grapho::gl3::ShaderProgram>& shader,
                     const WorldInfo&,
                     const LocalInfo&)>;

struct VarToStrVisitor
{
  std::string& m_name;

  std::stringstream m_ss;
  std::string operator()(const OptVar& var)
  {
    if (var.LastValue) {
      m_ss << "#define " << m_name;
    }
    return m_ss.str();
  }
  std::string operator()(const BoolVar& var)
  {
    if (var.LastValue) {
      m_ss << "#define " << m_name << " true";
    } else {
      m_ss << "#define " << m_name << " false";
    }
    return m_ss.str();
  }
  std::string operator()(const IntVar& var)
  {
    m_ss << "#define " << m_name << " " << var.LastValue;
    return m_ss.str();
  }
  std::string operator()(const FloatVar& var)
  {
    m_ss << "#define " << m_name << " " << var.LastValue;
    return m_ss.str();
  }
  std::string operator()(const StringVar& var)
  {
    m_ss << "#define " << m_name << " " << var.LastValue;
    return m_ss.str();
  }
};

struct ShaderMacro
{
  std::u8string Name;
  std::variant<OptVar, BoolVar, IntVar, FloatVar, StringVar> Value =
    OptVar{ [](auto, auto, auto) { return std::monostate{}; } };
  std::u8string Str() const
  {
    std::string name((const char*)Name.data(), Name.size());
    auto dst = std::visit(VarToStrVisitor{ name }, Value);
    return { (const char8_t*)dst.data(), dst.size() };
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
  ShaderMacro Selected;
};

struct ShaderFactory
{
  std::string SourceName;
  std::u8string Version;
  std::u8string Precision;
  std::vector<ShaderEnum> Enums;
  std::vector<std::u8string> Codes;
  std::vector<ShaderMacro> Macros;
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

using UniformVar =
  std::variant<IntVar, FloatVar, Vec3Var, Vec4Var, Mat3Var, Mat4Var>;

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

  std::unordered_map<std::string, UniformVar> UniformVarMap;
  std::vector<std::optional<UniformVar>> UniformVars;

  void Activate(const std::shared_ptr<ShaderSourceManager>& shaderSource,
                const WorldInfo& world,
                const LocalInfo& local,
                const gltfjson::tree::NodePtr& material)
  {
    if (!Compiled) {
      // execute mcaro
      for (auto& m : VS.Macros) {
        std::visit([&world, &local, &material](
                     auto& var) { var.Update(world, local, material); },
                   m.Value);
      }
      for (auto& m : FS.Macros) {
        std::visit([&world, &local, &material](
                     auto& var) { var.Update(world, local, material); },
                   m.Value);
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
