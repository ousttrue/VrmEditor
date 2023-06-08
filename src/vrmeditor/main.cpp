#include <Windows.h>
#include <docks/imlogger.h>
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

namespace plog {
template<class Formatter> // Typically a formatter is passed as a template
                          // parameter.
class ImLoggerAppender
  : public IAppender // All appenders MUST inherit IAppender interface.
{
public:
  virtual void write(const Record& record)
    PLOG_OVERRIDE // This is a method from IAppender that MUST be implemented.
  {
    util::nstring str = Formatter::format(
      record); // Use the formatter to get a string from a record.

    ImLogger::Instance().AddLog(record.getSeverity(), str);
  }
};
}

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

  // static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
  // plog::init(plog::verbose, &consoleAppender);
  static plog::ImLoggerAppender<plog::TxtFormatter> imLoggerAppender;
  plog::init(plog::debug,
             &imLoggerAppender); // Initialize the logger with our appender.

  //
  // auto& app = App::Instance();

  auto exe = GetExe();
  auto base = exe.parent_path().parent_path();
  app::SetShaderDir(base / "shaders");
  app::SetShaderChunkDir(base / "threejs_shader_chunks");

  // load user ~/.vrmeditor.lua
  auto user_conf = get_home() / ".vrmeditor.lua";
  if (std::filesystem::exists(user_conf)) {
    app::LoadLua(user_conf);
  }

  if (argc > 1) {
    std::string_view arg = argv[1];
    if (arg.ends_with(".lua")) {
      // prooject mode
      app::ProjectMode();
      app::LoadLua(argv[1]);
    } else {
      // viewermode
      app::LoadModel(argv[1]);
    }
  }

  return app::Run();
}
