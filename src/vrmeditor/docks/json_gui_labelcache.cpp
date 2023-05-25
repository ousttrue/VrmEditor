#include "json_gui_labelcache.h"

static std::u8string_view
GetIcon(const gltfjson::JsonPath& path)
{
  if (path.Size() == 2) {
    auto kind = path[1];
    if (kind == u8"extensions") {
      return u8" ";
    } else if (kind == u8"images" || kind == u8"textures" ||
               kind == u8"samplers") {
      return u8" ";
    } else if (kind == u8"bufferViews" || kind == u8"buffers") {
      return u8" ";
    } else if (kind == u8"accessors") {
      return u8" ";
    } else if (kind == u8"meshes" || kind == u8"skins") {
      return u8"󰕣 ";
    } else if (kind == u8"materials") {
      return u8" ";
    } else if (kind == u8"nodes" || kind == u8"scenes" || kind == u8"scene") {
      return u8"󰵉 ";
    } else if (kind == u8"animations") {
      return u8" ";
    } else if (kind == u8"asset") {
      return u8" ";
    } else if (kind == u8"extensionsUsed") {
      return u8" ";
    }
  }

  return u8"";
}

// static std::u8string
// LabelDefault(const gltfjson::tree::NodePtr& item, std::u8string_view
// jsonpath)
// {
//   std::stringstream ss;
//   // std::u8string key{ jsonpath.begin(), jsonpath.end() };
//   auto path = gltfjson::JsonPath(jsonpath);
//   // if (path.Size() == 2) {
//   //   auto kind = path[1];
//   //   if (kind == u8"extensions") {
//   //     ss << " ";
//   //   } else if (kind == u8"images" || kind == u8"textures" ||
//   //              kind == u8"samplers") {
//   //     ss << " ";
//   //   } else if (kind == u8"bufferViews" || kind == u8"buffers") {
//   //     ss << " ";
//   //   } else if (kind == u8"accessors") {
//   //     ss << " ";
//   //   } else if (kind == u8"meshes" || kind == u8"skins") {
//   //     ss << "󰕣 ";
//   //   } else if (kind == u8"materials") {
//   //     ss << " ";
//   //   } else if (kind == u8"nodes" || kind == u8"scenes" || kind ==
//   u8"scene") {
//   //     ss << "󰵉 ";
//   //   } else if (kind == u8"animations") {
//   //     ss << " ";
//   //   } else if (kind == u8"asset") {
//   //     ss << " ";
//   //   } else if (kind == u8"extensionsUsed") {
//   //     ss << " ";
//   //   }
//   // }
//
//   // auto item = scene->m_gltf.Json.at(nlohmann::json::json_pointer(key));
//   auto name = path.Back();
//   // std::string name{ _name.begin(), _name.end() };
//   if (auto object = item->Object()) {
//     auto found = object->find(u8"name");
//     if (found != object->end()) {
//       ss << gltfjson::tree::from_u8(name) << ": "
//          << gltfjson::tree::from_u8(found->second->U8String());
//     } else {
//       ss << gltfjson::tree::from_u8(name) << ": object";
//     }
//   } else if (auto array = item->Array()) {
//     ss << gltfjson::tree::from_u8(name) << ": [" << array->size() << "]";
//   } else {
//     ss << gltfjson::tree::from_u8(name) << ": " << *item;
//   }
//   return (const char8_t*)ss.str().c_str();
// }

LabelCacheManager::LabelCacheManager()
  : m_labelFactories({
      //
      // { "/images", LabelArray },
    })
{
}

static std::u8string_view
GetLastName(std::u8string_view src)
{
  auto pos = src.rfind('/');
  if (pos != std::string::npos) {
    return src.substr(pos + 1);
  } else {
    return src;
  }
}

const LabelCache&
LabelCacheManager::Get(const gltfjson::tree::NodePtr& item,
                       std::u8string_view jsonpath)
{
  auto found = m_labelCache.find({ jsonpath.begin(), jsonpath.end() });
  if (found != m_labelCache.end()) {
    return found->second;
  }

  LabelCache label;
  auto path = gltfjson::JsonPath(jsonpath);
  auto icon = GetIcon(path);
  label.Key += gltfjson::tree::from_u8(icon);
  label.Key += gltfjson::tree::from_u8(GetLastName(jsonpath));
  if (auto factory = MatchLabel(jsonpath)) {
    label.Value = (*factory)(item, jsonpath);
  } else {
    std::stringstream ss;
    ss << *item;
    label.Value = ss.str();
  }
  auto inserted =
    m_labelCache.insert({ { jsonpath.begin(), jsonpath.end() }, label });
  return inserted.first->second;
}
