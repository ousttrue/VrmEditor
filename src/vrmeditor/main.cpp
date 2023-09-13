#include <docks/imlogger.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <plog/Log.h>

#include "app.h"

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

#if defined(_WIN32) && defined(NDEBUG)
#include <windows.h>
int WINAPI
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
{
  auto argc = __argc;
  auto argv = __argv;
#else
int
main(int argc, char** argv)
{
#endif
  // #ifdef _WIN32
  //   RedirectIOToConsole();
  // #endif

  // static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
  // plog::init(plog::verbose, &consoleAppender);
  static plog::ImLoggerAppender<plog::TxtFormatter> imLoggerAppender;
  plog::init(plog::debug,
             &imLoggerAppender); // Initialize the logger with our appender.

  app::Run({ (const char**)argv + 1, (size_t)(argc - 1) });

  return 0;
}
