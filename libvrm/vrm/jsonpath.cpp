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
  for (auto jp : m_str | std::views::split(DELIMITER)) {
    ++size;
  }
  return size;
}

std::string_view
JsonPath::operator[](size_t i) const
{
  size_t size = 0;
  for (auto jp : m_str | std::views::split(DELIMITER)) {
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

bool
JsonPath::Match(std::string_view rhs) const
{
  auto ls = m_str.begin();
  auto rs = rhs.begin();

  for (; ls != m_str.end() && rs != rhs.end();) {
    auto le = std::find(ls, m_str.end(), DELIMITER);
    auto re = std::find(rs, rhs.end(), DELIMITER);

    std::string_view ll(ls, le);
    std::string_view rr(rs, re);

    if (ll == "*") {
      // wildcard
    } else if (ll != rr) {
      return false;
    }

    ls = le;
    if (ls != m_str.end()) {
      ++ls;
    }
    rs = re;
    if (rs != rhs.end()) {
      ++rs;
    }
  }

  return ls == m_str.end() && rs == rhs.end();
}

}
