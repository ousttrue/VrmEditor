#include <GL/glew.h>

#include "app.h"
#include "error_check.h"
#include <iostream>
#include <stdarg.h>
#include <stdio.h>

inline char const*
gl_error_string(GLenum const err) noexcept
{
  switch (err) {
    // opengl 2 errors (8)
    case GL_NO_ERROR:
      return "GL_NO_ERROR";

    case GL_INVALID_ENUM:
      return "GL_INVALID_ENUM";

    case GL_INVALID_VALUE:
      return "GL_INVALID_VALUE";

    case GL_INVALID_OPERATION:
      return "GL_INVALID_OPERATION";

    case GL_STACK_OVERFLOW:
      return "GL_STACK_OVERFLOW";

    case GL_STACK_UNDERFLOW:
      return "GL_STACK_UNDERFLOW";

    case GL_OUT_OF_MEMORY:
      return "GL_OUT_OF_MEMORY";

    case GL_TABLE_TOO_LARGE:
      return "GL_TABLE_TOO_LARGE";

    // opengl 3 errors (1)
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return "GL_INVALID_FRAMEBUFFER_OPERATION";

    // gles 2, 3 and gl 4 error are handled by the switch above
    default:
      assert(!"unknown error");
      return nullptr;
  }
}

void
GL_ErrorClear(const char* label)
{
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    // App::Instance().Log(LogLevel::Wran)
    //   << "clear: " << label << " => " << gl_error_string(err);
    //
    // std::cerr << "clear: " << label << " => " << gl_error_string(err);
  }
}

void
GL_ErrorCheck(const char* fmt, ...)
{
  static char m_buf[256];

  va_list arg_ptr;
  va_start(arg_ptr, fmt);
  vsnprintf(m_buf, sizeof(m_buf), fmt, arg_ptr);
  va_end(arg_ptr);

  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    App::Instance().Log(LogLevel::Wran)
      << m_buf << " => " << gl_error_string(err);

    std::cerr << m_buf << " => " << gl_error_string(err);
    assert(false);
  }
}
