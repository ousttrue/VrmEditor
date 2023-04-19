#pragma once
#include <optional>
#include <ranges>
#include <string_view>
#include <vector>

namespace libvrm {
class JsonPath
{
  std::string_view m_str;

public:
  JsonPath(std::string_view str);
  size_t Size() const;
  std::string_view operator[](size_t i) const;
  std::optional<int> GetInt(int n) const;

  bool operator==(const JsonPath& rhs) const { return m_str == rhs.m_str; }
  bool operator!=(const JsonPath& rhs) const { return m_str != rhs.m_str; }

  bool Match(std::string_view rhs) const;
};

}
