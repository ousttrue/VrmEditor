#pragma once
#include "info.h"
#include <functional>
#include <gltfjson.h>
#include <sstream>

namespace glr {

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

inline auto
ConstInt(int value)
{
  return IntVar{ [value](auto, auto, auto) { return value; } };
}
inline auto
ConstFloat(float value)
{
  return FloatVar{ [value](auto, auto, auto) { return value; } };
}
inline auto
ConstBool(bool value)
{
  return BoolVar{ [value](auto, auto, auto) { return value; } };
}
inline auto
Disable()
{
  return OptVar{ [](auto, auto, auto) { return std::nullopt; } };
}

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

}
