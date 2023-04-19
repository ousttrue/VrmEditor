#include "vrm/jsonpath.h"
#include <charconv>

namespace libvrm {

JsonPath::JsonPath(std::string_view str)
  : m_str(str)
{
}

size_t
JsonPath::Size() const
{
  size_t size = 0;
  for (auto jp : m_str | std::views::split('.')) {
    ++size;
  }
  return size;
}

std::string_view
JsonPath::operator[](size_t i) const
{
  size_t size = 0;
  for (auto jp : m_str | std::views::split('.')) {
    if (i == size) {
      return std::string_view{ jp };
    }
    ++size;
  }
  return {};
}

std::optional<int>
JsonPath::GetInt(int n) const
{
  auto i_view = (*this)[n];
  if (i_view.empty()) {
    return {};
  }
  int i;
  if (std::from_chars(i_view.data(), i_view.data() + i_view.size(), i).ec ==
      std::errc{}) {
    return i;
  }
  return {};
}

}
