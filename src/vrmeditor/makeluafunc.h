#pragma once
#include <filesystem>
#include <string>

#include <lua.hpp>
///
/// LuaGet
///
template<size_t I>
struct LuaGet
{
  template<typename T>
  static T Get(lua_State* L);

  template<>
  static bool Get<bool>(lua_State* L)
  {
    luaL_checktype(L, I + 1, LUA_TBOOLEAN);
    return lua_toboolean(L, I + 1);
  }

  template<>
  static int Get<int>(lua_State* L)
  {
    return (int)luaL_checkinteger(L, I + 1);
  }

  template<>
  static std::string Get<std::string>(lua_State* L)
  {
    const char* path = luaL_checkstring(L, I + 1);
    return std::string(path);
  }

  template<>
  static std::filesystem::path Get<std::filesystem::path>(lua_State* L)
  {
    const char* path = luaL_checkstring(L, I + 1);
    return std::filesystem::path(path);
  }
};

template<typename... AS, std::size_t... IS>
std::tuple<AS...>
GetTuple(lua_State* L, std::index_sequence<IS...>)
{
  auto t = std::make_tuple(LuaGet<IS>::template Get<AS>(L)...);
  return t;
}

///
/// LuaPush
///
template<typename T>
int
LuaPush(lua_State* L, const T& value);

template<>
int
LuaPush<bool>(lua_State* L, const bool& value)
{
  lua_pushboolean(L, value);
  return 1;
}

// template<>
// int
// LuaPush<bool>(
//   lua_State* L,
//   const std::expected<bool, std::string>& ret)
// {
//   if (ret) {
//     return LuaPush(L, *ret);
//   } else {
//     luaL_error(L, ret.error().c_str());
//     return {};
//   }
// }

template<void* F, typename R, typename C, typename... AS, std::size_t... IS>
constexpr lua_CFunction
_MakeLuaFunc(R (C::*)(AS...) const, std::index_sequence<IS...>)
{
  return [](lua_State* L) -> int {
    // unpack args
    auto args =
      GetTuple<std::decay_t<AS>...>(L, std::index_sequence_for<AS...>{});

    using Func = R (*)(AS...);

    if constexpr (std::is_void<R>::value) {
      ((Func)F)(std::get<IS>(args)...);
      return 0;
    } else {
      R r = ((Func)F)(std::get<IS>(args)...);
      LuaPush(L, r);
      return 1;
    }
  };
}

template<typename F, typename R, typename C, typename... AS>
constexpr lua_CFunction
_MakeLuaFunc(F f, R (C::*)(AS...) const)
{
  using Func = R (*)(AS...);
  constexpr Func _p = +f;
  constexpr auto p = (void*)_p;
  return _MakeLuaFunc<p>(&decltype(f)::operator(),
                         std::index_sequence_for<AS...>{});
}

template<typename F>
constexpr lua_CFunction
MakeLuaFunc(F f)
{
  return _MakeLuaFunc(f, &decltype(f)::operator());
}
