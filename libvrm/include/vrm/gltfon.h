#pragma once
// #include <expected>
// #include <vector>
// #include <string>
// #include <string_view>
#pragma warning(suppress : 5050)
import std.core;

// glTF Object Notation(JSON for glTF)
namespace gltfon {

struct JsonItem
{
  std::string_view View;
  std::vector<JsonItem> Children;

  JsonItem(std::string_view src)
    : View(src)
  {
  }

  bool IsNull() const { return View == "null"; }
  bool IsTrue() const { return View == "true"; }
  bool IsFalse() const { return View == "false"; }
  template<typename T>
  std::optional<T> Number() const
  {
    T out;
    if (auto [ptr, ec] =
          std::from_chars(View.data(), View.data() + View.size(), out);
        ec == std::errc{}) {
      return out;
    }
    return {};
  }
  std::optional<std::string_view> Unquoted() const
  {
    if (View.front() == '"' && View.back() == '"') {
      return View.substr(1, View.size() - 2);
    }
    return {};
  }
  size_t Size() const { return Children.size(); }
};

struct Reader
{
  const char* m_begin;
  const char* m_end;
  const char* m_current;
  Reader(std::string_view src)
    : m_begin(src.data())
    , m_end(src.data() + src.size())
    , m_current(m_begin)
  {
  }

  char operator*() const { return *m_current; }

  void SkipSpace()
  {
    for (; m_current != m_end && std::isspace(*m_current); ++m_current) {
    }
  }

  std::optional<std::string_view> Match(std::string_view src)
  {
    auto begin = m_current;
    auto it = m_current;
    auto rhs = src.begin();
    for (; it != m_end && rhs != src.end() && *it == *rhs; ++it, ++rhs) {
    }
    if (rhs != src.end()) {
      return {};
    }
    m_current = it;
    return std::string_view{ begin, it };
  }

  std::optional<std::string_view> Number()
  {
    auto begin = m_current;
    auto it = m_current;
    for (; it != m_end; ++it) {
      if (!std::isdigit(*it)) {
        break;
      }
    }
    if (it == m_current) {
      return {};
    }
    m_current = it;
    return std::string_view{ begin, it };
  }

  std::optional<std::string_view> String()
  {
    if (*m_current != '"') {
      return {};
    }
    auto begin = m_current;
    auto it = m_current;
    ++it;
    for (; it != m_end; ++it) {
      if (*it == '"') {
        ++it;
        m_current = it;
        return std::string_view{ begin, it };
      }
    }
    return {};
  }
};

std::expected<JsonItem, std::string>
Parse(std::string_view src)
{
  Reader r(src);
  r.SkipSpace();

  switch (*r) {
    case 'n':
      if (auto m = r.Match("null")) {
        return JsonItem(*m);
      }
      break;
    case 't':
      if (auto m = r.Match("true")) {
        return JsonItem(*m);
      }
      break;
    case 'f':
      if (auto m = r.Match("false")) {
        return JsonItem(*m);
      }
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '-':
      if (auto m = r.Number()) {
        return JsonItem(*m);
      }
      break;

    case '"':
      if (auto m = r.String()) {
        return JsonItem(*m);
      }
      break;

    // case '[':
    //   if (auto m = r.Array()) {
    //     return JsonItem(*m);
    //   }
    //   break;
  }

  return std::unexpected{ "invalid" };
}

}
