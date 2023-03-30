#include "luahost.h"
#include "app.h"
#include "windows_helper.h"
#include <iostream>

static int vrmeditor_load_model(lua_State *l) {
  auto path = luaL_checklstring(l, -1, nullptr);

  auto succeeded = App::Instance().LoadModel(path);
  lua_pushboolean(App::Instance().Lua()->state(), succeeded);
  return 1;
}

static int vrmeditor_load_motion(lua_State *l) {
  auto path = luaL_checklstring(l, -2, nullptr);
  auto scale = luaL_checknumber(l, -1);

  auto succeeded = App::Instance().LoadMotion(path, scale);
  lua_pushboolean(App::Instance().Lua()->state(), succeeded);
  return 1;
}

static int vrmeditor_add_asset_dir(lua_State *l) {
  auto name = luaL_checklstring(l, -2, nullptr);
  auto dir = luaL_checklstring(l, -1, nullptr);
  auto succeeded = App::Instance().AddAssetDir(name, dir);
  lua_pushboolean(App::Instance().Lua()->state(), succeeded);
  return 1;
}

static const struct luaL_Reg VrmEditorLuaModule[] = {
    {"load_model", vrmeditor_load_model},
    {"load_motion", vrmeditor_load_motion},
    {"add_asset_dir", vrmeditor_add_asset_dir},
    {NULL, NULL},
};

LuaEngine::LuaEngine() {
  L_ = luaL_newstate();
  luaL_openlibs(L_);
  luaL_register(L_, "vrmeditor", VrmEditorLuaModule);
}

LuaEngine::~LuaEngine() { lua_close(L_); }

void LuaEngine::eval(const std::string &script) {
  luaL_dostring(L_, script.c_str());
}

void LuaEngine::dofile(const std::filesystem::path &path) {

  auto mb = WideToMb(CP_OEMCP, path.c_str());

  auto ret = luaL_dofile(L_, mb.c_str());
  if (ret != 0) {
    std::cout << "luaL_dofile(): " << ret << std::endl;
    std::cout << lua_tostring(L_, -1) << std::endl;
  }
}
