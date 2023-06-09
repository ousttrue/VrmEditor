#pragma once
#include "json_gui.h"
#include <functional>
#include <gltfjson/jsonpath.h>
#include <list>
#include <string>

class JsonGuiFactoryManager
{
  gltfjson::JsonPathMap<JsonGuiItem> m_guiFactories;

  struct Cache
  {
    std::string Key;
    std::string Value;
    CreateGuiFunc Factory;
    std::string Description;
  };
  std::unordered_map<std::u8string, Cache> m_cacheMap;
  std::u8string m_jsonpath;
  Cache m_current;
  ShowGuiFunc m_editor;
  using OnUpdatedFunc = std::function<void(std::u8string_view)>;
  std::list<OnUpdatedFunc> m_onUpdatedCallbacks;

public:
  JsonGuiFactoryManager();

  void ClearAll()
  {
    m_cacheMap.clear();
    m_current = {};
    m_editor = {};
  }

  void ClearJsonPath(std::u8string_view jsonpath)
  {
    if (true /*IsChildOfRoot(jsonpath)*/) {
      // clear all descendants
      for (auto it = m_cacheMap.begin(); it != m_cacheMap.end();) {
        if (it->first.starts_with(jsonpath)) {
          // clear all descendants
          it = m_cacheMap.erase(it);
        } else {
          ++it;
        }
      }
    } else {
      for (auto it = m_cacheMap.begin(); it != m_cacheMap.end();) {
        gltfjson::JsonPath path(it->first);
        if (path.Match(jsonpath)) {
          it = m_cacheMap.erase(it);
        } else {
          ++it;
        }
      }
    }
  }

  bool IsSelected(std::u8string_view jsonpath) const
  {
    return jsonpath == m_jsonpath;
  }

  bool ShouldOpen(std::u8string_view jsonpath) const
  {
    if (jsonpath.size() == 1) {
      // "/"
      return true;
    }

    if (m_jsonpath.starts_with(jsonpath)) {
      if (m_jsonpath[jsonpath.size()] == u8'/') {
        return true;
      }
    }
    return false;
  }

  void Select(std::u8string_view jsonpath)
  {
    m_jsonpath = jsonpath;
    m_current = {};
    m_editor = {};
  }

  void ShowGui(const gltfjson::Root& root, const gltfjson::Bin& bin);

  void RaiseUpdated(std::u8string_view jsonpath)
  {
    for (auto& callback : m_onUpdatedCallbacks) {
      callback(jsonpath);
    }
  }

  void OnUpdated(const OnUpdatedFunc& callback)
  {
    m_onUpdatedCallbacks.push_back(callback);
  }

  inline std::u8string_view GetLastName(std::u8string_view src)
  {
    auto pos = src.rfind('/');
    if (pos != std::string::npos) {
      return src.substr(pos + 1);
    } else {
      return src;
    }
  }

  struct NodeTypeIconVisitor
  {
    const char* operator()(std::monostate) { return ""; }
    const char* operator()(bool) { return "✅"; }
    const char* operator()(float) { return "🔢"; }
    const char* operator()(const std::u8string&) { return "📄"; }
    const char* operator()(const gltfjson::tree::ArrayValue&) { return "[]"; }
    const char* operator()(const gltfjson::tree::ObjectValue&) { return "{}"; }
  };

  Cache& Get(std::u8string_view jsonpath, const gltfjson::tree::NodePtr& item)
  {
    auto found = m_cacheMap.find({ jsonpath.begin(), jsonpath.end() });
    if (found != m_cacheMap.end()) {
      return found->second;
    }

    Cache cache;
    // auto path = gltfjson::JsonPath(jsonpath);
    // std::u8string icon;
    auto factory = m_guiFactories.Match(jsonpath);
    // icon = found->Icon;
    // }
    if (factory) {
      cache.Key += gltfjson::from_u8(factory->Icon);
      cache.Factory = factory->Factory;
      cache.Description = factory->Description;
    } else {
      cache.Key += std::visit(NodeTypeIconVisitor{}, item->Var);
    }
    cache.Key += gltfjson::from_u8(GetLastName(jsonpath));
    auto object = item->Object();
    if (object && object->find(u8"name") != object->end()) {
      cache.Value = "{";
      cache.Value += gltfjson::from_u8((*object)[u8"name"]->U8String());
      cache.Value += "}";
    } else {
      std::stringstream ss;
      ss << *item;
      cache.Value = ss.str();
    }
    auto inserted = m_cacheMap.insert({
      { jsonpath.begin(), jsonpath.end() },
      cache,
    });
    return inserted.first->second;
  }
};
