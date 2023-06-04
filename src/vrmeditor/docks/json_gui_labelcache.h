#pragma once
#include <functional>
#include <gltfjson.h>
#include <gltfjson/jsonpath.h>
#include <list>
#include <optional>
#include <string>
#include <unordered_map>

struct LabelCache
{
  std::string Key;
  std::string Value;
};

class LabelCacheManager
{
  std::unordered_map<std::u8string, LabelCache> m_labelCache;
  gltfjson::JsonPathMap<std::u8string> m_iconMap;

public:
  LabelCacheManager();

  const LabelCache& Get(const gltfjson::tree::NodePtr& item,
                        std::u8string_view jsonpath);

  void Clear() { m_labelCache.clear(); }

  // static bool IsChildOfRoot(std::u8string_view jsonpath)
  // {
  //   gltfjson::JsonPath path(jsonpath);
  //   if (path.Size() == 3) {
  //     for (auto& name : gltfjson::ChildOfRootProperties) {
  //       if (path[1] == name) {
  //         return true;
  //       }
  //     }
  //   }
  //
  //   return false;
  // }

  void ClearCache(std::u8string_view jsonpath)
  {
    if (true /*IsChildOfRoot(jsonpath)*/) {
      // clear all descendants
      for (auto it = m_labelCache.begin(); it != m_labelCache.end();) {
        if (it->first.starts_with(jsonpath)) {
          // clear all descendants
          it = m_labelCache.erase(it);
        } else {
          ++it;
        }
      }
    } else {
      for (auto it = m_labelCache.begin(); it != m_labelCache.end();) {
        gltfjson::JsonPath path(it->first);
        if (path.Match(jsonpath)) {
          it = m_labelCache.erase(it);
        } else {
          ++it;
        }
      }
    }
  }
};
