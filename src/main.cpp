#include "app.h"
#include <Windows.h>
#include <fcntl.h> // _O_TEXT
#include <io.h>    // _open_osfhandle
#include <iostream>

// https://stackoverflow.com/questions/60328079/piping-console-output-from-winmain-when-running-from-a-console
void RedirectIOToConsole() {
  if (AttachConsole(ATTACH_PARENT_PROCESS) == false)
    return;

  HANDLE ConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  int SystemOutput = _open_osfhandle(intptr_t(ConsoleOutput), _O_TEXT);

  // check if output is a console and not redirected to a file
  if (isatty(SystemOutput) == false)
    return; // return if it's not a TTY

  FILE *COutputHandle = _fdopen(SystemOutput, "w");

  // Get STDERR handle
  HANDLE ConsoleError = GetStdHandle(STD_ERROR_HANDLE);
  int SystemError = _open_osfhandle(intptr_t(ConsoleError), _O_TEXT);
  FILE *CErrorHandle = _fdopen(SystemError, "w");

  // Get STDIN handle
  HANDLE ConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
  int SystemInput = _open_osfhandle(intptr_t(ConsoleInput), _O_TEXT);
  FILE *CInputHandle = _fdopen(SystemInput, "r");

  // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console
  // as well
  std::ios::sync_with_stdio(true);

  // Redirect the CRT standard input, output, and error handles to the console
  freopen_s(&CInputHandle, "CONIN$", "r", stdin);
  freopen_s(&COutputHandle, "CONOUT$", "w", stdout);
  freopen_s(&CErrorHandle, "CONOUT$", "w", stderr);

  // Clear the error state for each of the C++ standard stream objects.
  std::wcout.clear();
  std::cout.clear();
  std::wcerr.clear();
  std::cerr.clear();
  std::wcin.clear();
  std::cin.clear();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {

  RedirectIOToConsole();

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
