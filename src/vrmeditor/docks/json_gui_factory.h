#pragma once
#include "showgui.h"
#include <functional>
#include <gltfjson/jsonpath.h>
#include <list>

class JsonGuiFactoryManager
{
  std::u8string m_selected;
  gltfjson::JsonPathMap<CreateGuiFunc> m_guiFactories;
  ShowGuiFunc m_cache;
  using OnUpdatedFunc = std::function<void(std::u8string_view)>;
  std::list<OnUpdatedFunc> m_onUpdatedCallbacks;

public:
  JsonGuiFactoryManager();

  void Clear() { m_cache = {}; }

  bool IsSelected(std::u8string_view jsonpath) const
  {
    return jsonpath == m_selected;
  }

  bool ShouldOpen(std::u8string_view jsonpath) const
  {
    if (jsonpath.size() == 1) {
      // "/"
      return true;
    }

    if (m_selected.starts_with(jsonpath)) {
      if (m_selected[jsonpath.size()] == u8'/') {
        return true;
      }
    }
    return false;
  }

  void Select(std::u8string_view jsonpath)
  {
    m_selected = jsonpath;
    m_cache = {};
  }

  void ShowGui(const gltfjson::Root& root,
               const gltfjson::Bin& bin);

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
};
