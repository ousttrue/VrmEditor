#pragma once
#include <optional>
#include <string_view>
#include <vector>
#include <ranges>

namespace libvrm {
struct JsonPath
{
  std::string Str;

  size_t Size() const;
  std::string_view operator[](size_t i) const;
  std::optional<int> GetInt(int n) const;

  bool operator==(const JsonPath& rhs) const { return Str == rhs.Str; }
  bool operator!=(const JsonPath& rhs) const { return Str != rhs.Str; }
};

}
