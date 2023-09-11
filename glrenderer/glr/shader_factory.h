#pragma once
#include "shader_source.h"
#include "var.h"
#include <unordered_map>
#include <variant>

namespace glr {

struct ShaderMacro
{
  std::string Name;
  std::variant<OptVar, BoolVar, IntVar, FloatVar, StringVar> Value =
    OptVar{ [](auto, auto, auto) { return std::monostate{}; } };
  std::string Str() const
  {
    std::string name((const char*)Name.data(), Name.size());
    return std::visit(VarToStrVisitor{ name }, Value);
  }
};

struct ShaderEnum
{
  struct KeyValue
  {
    std::string Name;
    int Value;

    std::string Str() const
    {
      std::stringstream ss;
      ss << "#define " << Name << " " << Value;
      return ss.str();
    }
  };
  std::vector<KeyValue> Values;
  ShaderMacro Selected;
};

using ShaderMacroGroup = std::vector<ShaderMacro>;

struct ShaderFactory
{
  std::string SourceName;
  std::string Version;
  std::string Precision;
  std::vector<ShaderEnum> Enums;
  std::vector<std::string> Codes;
  std::unordered_map<std::string, ShaderMacroGroup> MacroGroups;
  std::string SourceExpanded;
  std::string FullSource;

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

  std::string Expand(const std::shared_ptr<ShaderSourceManager>& shaderSource)
  {
    if (SourceName.empty()) {
      return {};
    }
    FullSource.clear();

    auto src = shaderSource->Get(SourceName);
    SourceExpanded = src->Source;

    FullSource += Version;
    FullSource.push_back('\n');

    if (Precision.size()) {
      FullSource += "precision ";
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
