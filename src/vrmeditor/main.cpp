#include "app.h"
#include "fs_util.h"
#include <iostream>

// #ifdef _WIN32
// #include "windows_helper.h"
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

  auto& app = App::Instance();

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
