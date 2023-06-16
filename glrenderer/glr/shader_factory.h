#pragma once
#include "shader_source.h"
#include "var.h"
#include <unordered_map>
#include <variant>

namespace glr {

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
      ss << "#define " << gltfjson::from_u8(Name) << " " << Value;
      auto str = ss.str();
      return { (const char8_t*)str.data(), str.size() };
    }
  };
  std::vector<KeyValue> Values;
  ShaderMacro Selected;
};

using ShaderMacroGroup = std::vector<ShaderMacro>;

struct ShaderFactory
{
  std::string SourceName;
  std::u8string Version;
  std::u8string Precision;
  std::vector<ShaderEnum> Enums;
  std::vector<std::u8string> Codes;
  std::unordered_map<std::string, ShaderMacroGroup> MacroGroups;
  std::u8string SourceExpanded;
  std::u8string FullSource;

  void Update(const WorldInfo& world, const LocalInfo& local, const Gltf& gltf)
  {
    for (auto& e : Enums) {
      std::visit(
        [&world, &local, &gltf](auto& var) { var.Update(world, local, gltf); },
        e.Selected.Value);
    }
    for (auto& g : MacroGroups) {
      for (auto& m : g.second) {
        std::visit([&world, &local, &gltf](
                     auto& var) { var.Update(world, local, gltf); },
                   m.Value);
      }
    }
  }

  std::u8string Expand(const std::shared_ptr<ShaderSourceManager>& shaderSource)
  {
    FullSource.clear();

    auto src = shaderSource->Get(SourceName);
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

    for (auto& g : MacroGroups) {
      for (auto& m : g.second) {
        FullSource += m.Str();
        FullSource.push_back('\n');
      }
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

}
