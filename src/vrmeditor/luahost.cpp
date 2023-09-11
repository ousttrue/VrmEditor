#include "luahost.h"
#include "app.h"
#include "docks/dockspace.h"
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

int
luaopen_vrmeditor(lua_State* L)
{
  constexpr struct luaL_Reg VrmEditorLuaModule[] = {
    { "imgui_load_ini", MakeLuaFunc([](const std::string& ini) {
        Gui::Instance().LoadState(ini);
      }) },
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
        Gui::Instance().SetFontSize(font_size);
      }) },
    { "add_font",
      MakeLuaFunc([](const std::filesystem::path& path,
                     const std::string type) {
        if (type == "emoji") {
          return Gui::Instance().m_fonts.push_back(
            FontSetting::EmojiFont(path));
        } else if (type == "icon") {
          return Gui::Instance().m_fonts.push_back(FontSetting::NerdFont(path));
        } else {
          return Gui::Instance().m_fonts.push_back(
            FontSetting::JapaneseFont(path));
        }
      }) },
    // asset
    { "load_model", MakeLuaFunc([](const std::filesystem::path& path) {
        return app::TaskLoadModel(path);
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
        DockSpaceManager::Instance().SetDockVisible(name, visible);
      }) },
    { "load_hdr", MakeLuaFunc([](const std::filesystem::path& path) {
        app::TaskLoadHdr(path);
      }) },
    { "set_shaderpath", MakeLuaFunc([](const std::filesystem::path& path) {
        app::SetShaderDir(path);
      }) },
    { "set_shader_chunk_path",
      MakeLuaFunc([](const std::filesystem::path& path) {
        glr::SetShaderChunkDir(path);
      }) },
    { nullptr, nullptr },
  };
  // luaL_register(m_lua, "vrmeditor", VrmEditorLuaModule);
  luaL_newlib(L, VrmEditorLuaModule);

  return 1;
}

struct LuaEngineImpl
{
  lua_State* m_lua = nullptr;

  LuaEngineImpl()
    : m_lua(luaL_newstate())
  {
    luaL_openlibs(m_lua);
    luaL_requiref(m_lua, "vrmeditor", luaopen_vrmeditor, 1);
  }
  ~LuaEngineImpl() { lua_close(m_lua); }

  bool Eval(const std::string& script)
  {
    auto ret = luaL_dostring(m_lua, script.c_str());
    if (ret != 0) {
      std::string msg = lua_tostring(m_lua, -1);
      lua_pop(m_lua, 1); // pop error message
      // return std::unexpected{ msg };
      return {};
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

bool
LuaEngine::Eval(std::string_view script)
{
  return m_impl->Eval({ script.begin(), script.end() });
}

void
LuaEngine::DoFile(const std::filesystem::path& path)
{
  m_impl->DoFile(path);
}
