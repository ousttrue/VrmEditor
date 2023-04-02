#include "app.h"
#include <iostream>

#ifdef _WIN32
#include "windows_helper.h"
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
#else
int main(int __argc, char **__argv)
#endif
{

#ifdef _WIN32
  RedirectIOToConsole();
#endif

  auto &app = App::Instance();

  if (__argc > 1) {
    std::string_view arg = __argv[1];
    if (arg.ends_with(".lua")) {
      app.LoadLua(__argv[1]);
    } else {
      app.LoadModel(__argv[1]);
    }
  }

  return app.Run();
}
