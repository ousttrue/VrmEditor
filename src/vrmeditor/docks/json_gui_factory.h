#pragma once
#include "showgui.h"
#include <gltfjson/jsonpath.h>
#include <list>

struct JsonGuiFactory
{
  std::u8string m_match;
  CreateGuiFunc Factory;

  bool Match(std::u8string_view jsonpath)
  {
    return gltfjson::JsonPath(m_match).Match(jsonpath);
  }
};

class JsonGuiFactoryManager
{
  std::u8string m_selected;
  std::list<JsonGuiFactory> m_guiFactories;
  ShowGuiFunc m_cache;

public:
  JsonGuiFactoryManager();

  void Clear() { m_cache = {}; }

  bool IsSelected(std::u8string_view jsonpath) const
  {
    return jsonpath == m_selected;
  }

  bool ShouldOpen(std::u8string_view jsonpath) const
  {
    return jsonpath.size() < m_selected.size() &&
           m_selected.starts_with(jsonpath);
  }

  void Select(std::u8string_view jsonpath)
  {
    m_selected = jsonpath;
    m_cache = {};
  }

  std::optional<CreateGuiFunc> MatchGui(std::u8string_view jsonpath)
  {
    for (auto& f : m_guiFactories) {
      if (f.Match(jsonpath)) {
        return f.Factory;
      }
    }
    // not found
    return {};
  }

  void ShowGui(const gltfjson::typing::Root& root,
               const gltfjson::typing::Bin& bin);
};
