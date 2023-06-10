#pragma once
#include <stdarg.h>
#include <stdio.h>

class PrintfBuffer
{
  char m_buf[256];

public:
  const char* Printf(const char* fmt, ...)
  {
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    vsnprintf(m_buf, sizeof(m_buf), fmt, arg_ptr);
    va_end(arg_ptr);
    return m_buf;
  }
};
