#include "vrm/jsonpath.h"
#include <charconv>

namespace libvrm {

JsonPath::JsonPath(std::string_view str)
{
  SetStr(str);
}

JsonPath&
JsonPath::operator=(const JsonPath& rhs)
{
  SetStr(rhs.Str);
  return *this;
}

void
JsonPath::SetStr(std::string_view str)
{
  Str = str;
  Separated.clear();
  for (auto jp : Str | std::views::split('.')) {
    Separated.push_back(std::string_view(jp));
  }
}

std::optional<int>
JsonPath::GetInt(int n) const
{
  auto i_view = Separated[n];
  int i;
  if (std::from_chars(i_view.data(), i_view.data() + i_view.size(), i).ec ==
      std::errc{}) {
    return i;
  }
  return {};
}

}
