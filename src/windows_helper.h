#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <fcntl.h> // _O_TEXT
#include <io.h>    // _open_osfhandle
#include <string>

inline std::string WideToMb(uint32_t cp, const wchar_t *src) {
  auto l = WideCharToMultiByte(cp, 0, src, -1, nullptr, 0, nullptr, nullptr);
  std::string dst;
  dst.resize(l);
  l = WideCharToMultiByte(cp, 0, src, -1, dst.data(), l, nullptr, nullptr);
  dst.push_back(0);
  return dst;
}

// https://stackoverflow.com/questions/60328079/piping-console-output-from-winmain-when-running-from-a-console
inline void RedirectIOToConsole() {
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
