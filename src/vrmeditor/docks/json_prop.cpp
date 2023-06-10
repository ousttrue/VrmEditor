#include "json_prop.h"

std::u8string
JsonProp::Value(const gltfjson::tree::NodePtr& item) const
{
  if (item) {
    std::u8string name;
    if (auto obj = item->Object()) {
      if (auto p = item->Get(u8"name")) {
        name = p->U8String();
      }
    }
    std::stringstream ss;
    if (name.size()) {
      ss << "{" << gltfjson::from_u8(name) << "}";
    } else {
      ss << *item;
    }
    auto str = ss.str();
    return std::u8string{ (const char8_t*)str.data(), str.size() };
  } else {
    return u8"";
  }
}
