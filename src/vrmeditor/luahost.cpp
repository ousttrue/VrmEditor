#include "luahost.h"
#include "app.h"
#include "docks/gui.h"
#include "makeluafunc.h"
#include <filesystem>
#include <iostream>
#include <type_traits>

static int
vrmeditor_add_human_map(lua_State* l)
{
  int stackSize = lua_gettop(l);
  if (stackSize < 1) {
    return 0;
  }
  if (lua_type(l, 1) != LUA_TTABLE) {
    return 0;
  }

  auto map = App::Instance().AddHumanBoneMap();
  lua_pushnil(l);
  while (lua_next(l, -2)) {
    if (auto value = lua_tostring(l, -1)) {
      map->Add(lua_tostring(l, -2), value);
    }
    lua_pop(l, 1);
  }
  lua_pop(l, 1);

  return 0;
}

struct LuaEngineImpl
{
  lua_State* m_lua = nullptr;

  LuaEngineImpl()
    : m_lua(luaL_newstate())
  {
    luaL_openlibs(m_lua);

    constexpr struct luaL_Reg VrmEditorLuaModule[] = {
      // font settings
      { "set_font_size", MakeLuaFunc([](int font_size) {
          App::Instance().GetGui()->SetFontSize(font_size);
        }) },
      { "set_font", MakeLuaFunc([](const std::filesystem::path& path) {
          return App::Instance().GetGui()->SetFont(path);
        }) },
      { "add_japanese_font", MakeLuaFunc([](const std::filesystem::path& path) {
          return App::Instance().GetGui()->AddJapaneseFont(path);
        }) },
      { "add_icon_font", MakeLuaFunc([](const std::filesystem::path& path) {
          return App::Instance().GetGui()->AddIconFont(path);
        }) },
      // asset
      { "load_model", MakeLuaFunc([](const std::filesystem::path& path) {
          return App::Instance().LoadModel(path);
        }) },
      { "load_motion", MakeLuaFunc([](const std::filesystem::path& path) {
          return App::Instance().LoadMotion(path);
        }) },
      { "add_asset_dir",
        MakeLuaFunc(
          [](const std::string& name, const std::filesystem::path& dir) {
            App::Instance().AddAssetDir(name, dir);
          }) },
      { "add_human_map", vrmeditor_add_human_map },
      { nullptr, nullptr },
    };
    luaL_register(m_lua, "vrmeditor", VrmEditorLuaModule);
  }
  ~LuaEngineImpl() { lua_close(m_lua); }

  std::expected<bool, std::string> Eval(const std::string& script)
  {
    auto ret = luaL_dostring(m_lua, script.c_str());
    if (ret != 0) {
      std::string msg = lua_tostring(m_lua, -1);
      lua_pop(m_lua, 1); // pop error message
      return std::unexpected{ msg };
    }
    return true;
  }

  std::expected<bool, std::string> DoFile(const std::filesystem::path& path)
  {
    auto mb = path.u8string();
    auto ret = luaL_dofile(m_lua, (const char*)mb.c_str());
    if (ret != 0) {
      std::string msg = lua_tostring(m_lua, -1);
      lua_pop(m_lua, 1); // pop error message
      // std::cout << "luaL_dofile(): " << ret << std::endl;
      // std::cout << lua_tostring(m_lua, -1) << std::endl;
      return std::unexpected{ msg };
    }
    return true;
  }
};

LuaEngine::LuaEngine()
  : m_impl(new LuaEngineImpl)
{
}

LuaEngine::~LuaEngine()
{
  delete m_impl;
}

std::expected<bool, std::string>
LuaEngine::Eval(std::string_view script)
{
  return m_impl->Eval({ script.begin(), script.end() });
}

std::expected<bool, std::string>
LuaEngine::DoFile(const std::filesystem::path& path)
{
  return m_impl->DoFile(path);
}
