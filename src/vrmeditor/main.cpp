#include <Windows.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <plog/Log.h>

#include "app.h"
#include "fs_util.h"
#include <iostream>
#ifdef _WIN32
#include "windows_helper.h"
#else
#endif

// int WINAPI
// WinMain(HINSTANCE hInstance,
//         HINSTANCE hPrevInstance,
//         LPSTR lpCmdLine,
//         int nCmdShow)
// #else
int
main(int argc, char** argv)
// #endif
{
  // #ifdef _WIN32
  //   RedirectIOToConsole();
  // #endif

  static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
  plog::init(plog::verbose, &consoleAppender);

  // Log severity levels are printed in different colors.
  PLOG_VERBOSE << "This is a VERBOSE message";
  PLOG_DEBUG << "This is a DEBUG message";
  PLOG_INFO << "This is an INFO message";
  PLOG_WARNING << "This is a WARNING message";
  PLOG_ERROR << "This is an ERROR message";
  PLOG_FATAL << "This is a FATAL message";

  auto& app = App::Instance();

  auto exe = GetExe();
  auto base = exe.parent_path().parent_path();
  app.SetShaderDir(base / "shaders");
  app.SetShaderChunkDir(base / "threejs_shader_chunks");

  // load user ~/.vrmeditor.lua
  auto user_conf = get_home() / ".vrmeditor.lua";
  if (std::filesystem::exists(user_conf)) {
    app.LoadLua(user_conf);
  }

  if (argc > 1) {
    std::string_view arg = argv[1];
    if (arg.ends_with(".lua")) {
      // prooject mode
      app.ProjectMode();
      app.LoadLua(argv[1]);
    } else {
      // viewermode
      app.LoadModel(argv[1]);
    }
  }

  return app.Run();
}
