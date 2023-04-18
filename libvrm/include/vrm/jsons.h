#pragma once
#include <assert.h>
#include <functional>
#include <string_view>

// json stream
//
// TODO:
// use std::to_chars if clang implements
//
namespace libvrm::jsons {
using WriteFunc = std::function<void(std::string_view)>;

enum StackFlags
{
  None = 0,
  Root = 0x01,
  Array = 0x02,
  Object = 0x04,
  Comma = 0x08,
  Collon = 0x10,
};

class Writer
{
  WriteFunc m_writer;
  char m_buf[1024];
  StackFlags m_stack[124] = { StackFlags::Root };
  int m_depth = 1;

public:
  Writer(const WriteFunc& writer)
    : m_writer(writer)
  {
  }

  ~Writer() { assert(m_depth == 1); }

  Writer(const Writer&) = delete;
  Writer& operator=(const Writer&) = delete;

  void push(std::string_view str)
  {
    if (m_stack[m_depth - 1] & StackFlags::Comma) {
      assert((m_stack[m_depth - 1] & StackFlags::Root) == 0);
      if (m_stack[m_depth - 1] & StackFlags::Collon) {
        m_writer(":");
        m_stack[m_depth - 1] =
          static_cast<StackFlags>(m_stack[m_depth - 1] & ~StackFlags::Collon);
      } else {
        m_writer(",");
      }
    } else {
      m_stack[m_depth - 1] =
        static_cast<StackFlags>(m_stack[m_depth - 1] | StackFlags::Comma);
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

  void null() { push("null"); }

  void value(bool is_true)
  {
    if (is_true) {
      push("true");
    } else {
      push("false");
    }
  }

  void value(int value)
  {
    auto len = snprintf(m_buf, sizeof(m_buf), "%d", value);
    push({ m_buf, m_buf + len });
  }

  void value(uint32_t value)
  {
    auto len = snprintf(m_buf, sizeof(m_buf), "%u", value);
    push({ m_buf, m_buf + len });
  }

  void value(size_t value)
  {
    auto len = snprintf(m_buf, sizeof(m_buf), "%zu", value);
    push({ m_buf, m_buf + len });
  }

  void value(float value)
  {
    auto len = snprintf(m_buf, sizeof(m_buf), "%f", value);
    push({ m_buf, m_buf + len });
  }

  void value(std::string_view str)
  {
    // TODO: quote
    push("\"");
    m_writer(str);
    m_writer("\"");
  }

  void value(const char* str) { value(std::string_view(str)); }

  void key(std::string_view str)
  {
    value(str);
    m_stack[m_depth - 1] =
      static_cast<StackFlags>(m_stack[m_depth - 1] | StackFlags::Collon);
  }
};

}
