#pragma once
#include <optional>
#include <string_view>
#include <vector>
#include <ranges>

namespace libvrm {
struct JsonPath
{
  std::string Str;

private:
  std::vector<std::string_view> Separated;
  void SetStr(std::string_view str);

public:
  JsonPath() {}
  JsonPath(std::string_view str);
  JsonPath(const JsonPath& rhs) { *this = rhs; }
  JsonPath& operator=(const JsonPath& rhs);
  size_t Size() const { return Separated.size(); }
  std::string_view operator[](size_t i) const { return Separated[i]; }
  std::optional<int> GetInt(int n) const;

  bool operator==(const JsonPath& rhs) const { return Str == rhs.Str; }
  bool operator!=(const JsonPath& rhs) const { return Str != rhs.Str; }
};

}
