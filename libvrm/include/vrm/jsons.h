#pragma once
#include <assert.h>
#include <functional>
#include <string_view>

// json stream
//
// TODO:
// use std::to_chars if clang implements
//
namespace jsons {
using WriteFunc = std::function<void(std::string_view)>;

enum StackFlags
{
  None = 0,
  Array = 0x1,
  Object = 0x2,
  Comma = 0x4,
  Root = 0x8,
};

class Writer
{
  WriteFunc m_writer;
  char m_buf[1024];
  StackFlags m_stack[124];
  int m_depth = 0;

public:
  Writer(const WriteFunc& writer)
    : m_writer(writer)
  {
  }

  void push(std::string_view str)
  {
    // root
    if (m_depth == 0) {
      m_stack[m_depth] = StackFlags::Root;
      ++m_depth;
    } else {
      assert((m_stack[0] & StackFlags::Root) == 0);

      if (m_stack[m_depth - 1] & StackFlags::Comma) {
        m_writer(",");
      } else {
        m_stack[m_depth - 1] =
          static_cast<StackFlags>(m_stack[m_depth - 1] | StackFlags::Comma);
      }
    }
    m_writer(str);
  }

  void array_open()
  {
    push("[");
    m_stack[m_depth++] = StackFlags::Array;
  }

  void array_close()
  {
    assert(m_depth > 0);
    assert(m_stack[m_depth - 1] & StackFlags::Array);
    --m_depth;
    m_writer("]");
  }

  void object_open()
  {
    push("{");
    m_stack[m_depth++] = StackFlags::Object;
  }

  void object_close()
  {
    assert(m_depth > 0);
    assert(m_stack[m_depth - 1] & StackFlags::Object);
    --m_depth;
    m_writer("}");
  }

  void write_null() { push("null"); }

  void write(bool is_true)
  {
    if (is_true) {
      push("true");
    } else {
      push("false");
    }
  }

  void write(int value)
  {
    auto len = snprintf(m_buf, sizeof(m_buf), "%d", value);
    push({ m_buf, m_buf + len });
  }

  void write(float value)
  {
    auto len = snprintf(m_buf, sizeof(m_buf), "%f", value);
    push({ m_buf, m_buf + len });
  }

  void write(std::string_view str)
  {
    // TODO: quote
    push("\"");
    m_writer(str);
    m_writer("\"");
  }

  void write(const char* str) { write(std::string_view(str)); }
};

}
