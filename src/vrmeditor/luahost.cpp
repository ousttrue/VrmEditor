#include "luahost.h"
#include "app.h"
#include "docks/gui.h"
#include "glr/gl3renderer.h"
#include "humanpose/humanpose_stream.h"
#include "makeluafunc.h"
#include "platform.h"
#include <filesystem>
#include <iostream>
#include <plog/Log.h>
#include <type_traits>

static int
vrmeditor_add_human_map(lua_State* L)
{
  int stackSize = lua_gettop(L);
  if (stackSize < 1) {
    return 0;
  }
  if (lua_type(L, 1) != LUA_TTABLE) {
    return 0;
  }

  auto map = humanpose::HumanPoseStream::Instance().AddHumanBoneMap();
  lua_pushnil(L);
  while (lua_next(L, -2)) {
    if (auto value = lua_tostring(L, -1)) {
      map->Add(lua_tostring(L, -2), value);
    }
    lua_pop(L, 1);
  }
  lua_pop(L, 1);

  return 0;
}

static int
vrmeditor_load_imnodes_links(lua_State* L)
{
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
      { "imgui_load_ini",
        MakeLuaFunc([](const std::string& ini) { app::LoadImGuiIni(ini); }) },
      { "imnodes_load_ini", MakeLuaFunc([](const std::string& ini) {
          humanpose::HumanPoseStream::Instance().LoadIni(ini);
        }) },
      { "imnodes_add_link", MakeLuaFunc([](int start, int end) {
          humanpose::HumanPoseStream::Instance().TryCreateLink(start, end);
        }) },
      { "set_window_size",
        MakeLuaFunc([](int width, int height, bool is_maximized) {
          Platform::Instance().SetWindowSize(width, height, is_maximized);
        }) },
      // font settings
      { "set_font_size", MakeLuaFunc([](int font_size) {
          Gui::Instance().FontSize = font_size;
        }) },
      { "set_font", MakeLuaFunc([](const std::filesystem::path& path) {
          return Gui::Instance().SetFont(path);
        }) },
      { "add_japanese_font", MakeLuaFunc([](const std::filesystem::path& path) {
          return Gui::Instance().AddJapaneseFont(path);
        }) },
      { "add_icon_font", MakeLuaFunc([](const std::filesystem::path& path) {
          return Gui::Instance().AddIconFont(path);
        }) },
      // asset
      { "load_model", MakeLuaFunc([](const std::filesystem::path& path) {
          return app::LoadModel(path);
        }) },
      { "load_motion", MakeLuaFunc([](const std::filesystem::path& path) {
          return humanpose::HumanPoseStream::Instance().LoadMotion(path);
        }) },
      { "add_asset_dir",
        MakeLuaFunc(
          [](const std::string& name, const std::filesystem::path& dir) {
            app::AddAssetDir(name, dir);
          }) },
      { "add_human_map", vrmeditor_add_human_map },
      { "show_dock", MakeLuaFunc([](const std::string& name, bool visible) {
          app::ShowDock(name, visible);
        }) },
      { "load_pbr", MakeLuaFunc([](const std::filesystem::path& path) {
          app::LoadPbr(path);
        }) },
      { "set_shaderpath", MakeLuaFunc([](const std::filesystem::path& path) {
          app::SetShaderDir(path);
        }) },
      { "set_shader_chunk_path",
        MakeLuaFunc([](const std::filesystem::path& path) {
          app::SetShaderChunkDir(path);
        }) },
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

  void DoFile(const std::filesystem::path& path)
  {
    auto mb = path.u8string();
    auto ret = luaL_dofile(m_lua, (const char*)mb.c_str());
    if (ret != 0) {
      std::string msg = lua_tostring(m_lua, -1);
      lua_pop(m_lua, 1); // pop error message
      // std::cout << "luaL_dofile(): " << ret << std::endl;
      // std::cout << lua_tostring(m_lua, -1) << std::endl;
      // return std::unexpected{ msg };
      PLOG_ERROR << msg;
    }
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

void
LuaEngine::DoFile(const std::filesystem::path& path)
{
  m_impl->DoFile(path);
}
