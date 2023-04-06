#include "luahost.h"
#include "app.h"
#include "docks/gui.h"
#include <iostream>

// vrmeditor.set_font_size(value) -> void
static int
vrmeditor_set_font_size(lua_State* l)
{
  int stackSize = lua_gettop(l);
  if (stackSize < 1) {
    return 0;
  }
  auto size = luaL_checknumber(l, 1);
  App::Instance().GetGui()->SetFontSize(static_cast<int>(size));
  return 0;
}

// vrmeditor.set_font("font.ttf") -> bool
static int
vrmeditor_set_font(lua_State* l)
{
  int stackSize = lua_gettop(l);
  if (stackSize < 1) {
    return 0;
  }
  auto path = luaL_checklstring(l, 1, nullptr);
  auto succeeded = App::Instance().GetGui()->SetFont(path);
  lua_pushboolean(App::Instance().Lua()->state(), succeeded);
  return 1;
}

// vrmeditor.add_japanese_font("font.ttf") -> bool
static int
vrmeditor_add_japanese_font(lua_State* l)
{
  int stackSize = lua_gettop(l);
  if (stackSize < 1) {
    return 0;
  }
  auto path = luaL_checklstring(l, 1, nullptr);
  auto succeeded = App::Instance().GetGui()->AddJapaneseFont(path);
  lua_pushboolean(App::Instance().Lua()->state(), succeeded);
  return 1;
}

// vrmeditor.add_icon_font("font.ttf") -> bool
static int
vrmeditor_add_icon_font(lua_State* l)
{
  int stackSize = lua_gettop(l);
  if (stackSize < 1) {
    return 0;
  }
  auto path = luaL_checklstring(l, 1, nullptr);
  auto succeeded = App::Instance().GetGui()->AddIconFont(path);
  lua_pushboolean(App::Instance().Lua()->state(), succeeded);
  return 1;
}

// vrmeditor.load_model("model.glb") -> bool
static int
vrmeditor_load_model(lua_State* l)
{
  int stackSize = lua_gettop(l);
  if (stackSize < 1) {
    return 0;
  }
  auto path = luaL_checklstring(l, 1, nullptr);
  auto succeeded = App::Instance().LoadModel(path);
  lua_pushboolean(App::Instance().Lua()->state(), succeeded);
  return 1;
}

// vrmeditor.load_motion("model.glb", scaling) -> bool
static int
vrmeditor_load_motion(lua_State* l)
{
  int stackSize = lua_gettop(l);
  if (stackSize < 2) {
    return 0;
  }
  auto path = luaL_checklstring(l, 1, nullptr);
  auto scale = luaL_checknumber(l, 2);

  auto succeeded = App::Instance().LoadMotion(path, scale);
  lua_pushboolean(App::Instance().Lua()->state(), succeeded);
  return 1;
}

// vrmeditor.add_asset_dir("name", path_to_dir") -> bool
static int
vrmeditor_add_asset_dir(lua_State* l)
{
  int stackSize = lua_gettop(l);
  if (stackSize < 2) {
    return 0;
  }
  auto name = luaL_checklstring(l, 1, nullptr);
  auto dir = luaL_checklstring(l, 2, nullptr);
  auto succeeded = App::Instance().AddAssetDir(name, dir);
  lua_pushboolean(App::Instance().Lua()->state(), succeeded);
  return 1;
}

// vrmeditor.add_asset_dir({joint0 = bone0, joint1=bone1,}) -> void
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

static const struct luaL_Reg VrmEditorLuaModule[] = {
  // font settings
  { "set_font_size", vrmeditor_set_font_size },
  { "set_font", vrmeditor_set_font },
  { "add_japanese_font", vrmeditor_add_japanese_font },
  { "add_icon_font", vrmeditor_add_icon_font },
  // asset
  { "load_model", vrmeditor_load_model },
  { "load_motion", vrmeditor_load_motion },
  { "add_asset_dir", vrmeditor_add_asset_dir },
  { "add_human_map", vrmeditor_add_human_map },
  { nullptr, nullptr },
};

LuaEngine::LuaEngine()
{
  L_ = luaL_newstate();
  luaL_openlibs(L_);
  luaL_register(L_, "vrmeditor", VrmEditorLuaModule);
}

LuaEngine::~LuaEngine()
{
  lua_close(L_);
}

void
LuaEngine::eval(const std::string& script)
{
  luaL_dostring(L_, script.c_str());
}

void
LuaEngine::dofile(const std::filesystem::path& path)
{
  auto mb = path.u8string();
  auto ret = luaL_dofile(L_, (const char*)mb.c_str());
  if (ret != 0) {
    std::cout << "luaL_dofile(): " << ret << std::endl;
    std::cout << lua_tostring(L_, -1) << std::endl;
  }
}
