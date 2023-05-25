#pragma once
#include <functional>
#include <gltfjson.h>
#include <gltfjson/jsonpath.h>
#include <list>
#include <optional>
#include <string>
#include <unordered_map>

using CreateLabelFunc =
  std::function<std::string(const gltfjson::tree::NodePtr& json,
                            std::u8string_view jsonpath)>;

struct LabelCache
{
  std::string Key;
  std::string Value;
};

struct JsonLabelFactory
{
  std::u8string m_match;
  CreateLabelFunc Factory;

  bool Match(std::u8string_view jsonpath)
  {
    return gltfjson::JsonPath(m_match).Match(jsonpath);
  }
};

class LabelCacheManager
{
  std::list<JsonLabelFactory> m_labelFactories;
  std::unordered_map<std::u8string, LabelCache> m_labelCache;

public:
  LabelCacheManager();
  std::optional<CreateLabelFunc> MatchLabel(std::u8string_view jsonpath)
  {
    for (auto& f : m_labelFactories) {
      if (f.Match(jsonpath)) {
        return f.Factory;
      }
    }
    // not found
    return {};
  }

  const LabelCache& Get(const gltfjson::tree::NodePtr& item,
                        std::u8string_view jsonpath);

  void Clear() { m_labelCache.clear(); }
};
