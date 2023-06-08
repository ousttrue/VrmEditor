#include "json_gui_labelcache.h"

LabelCacheManager::LabelCacheManager()
  : m_iconMap({
      //
      { u8"/extensions/VRM", u8"🌟" },
      { u8"/extensions/VRM/meta", u8"📄" },
      { u8"/extensions/VRM/humanoid", u8"👤" },
      { u8"/extensions/VRM/blendShapeMaster", u8"😀" },
      { u8"/extensions/VRM/firstPerson", u8"👀" },
      { u8"/extensions/VRM/secondaryAnimation", u8"🔗" },
      { u8"/extensions/VRM/materialProperties", u8"🎨" },
      // { u8"/extensions/VRM/*", u8"🌟" },
      //
      { u8"/extensions/VRMC_vrm", u8"🌟" },
      { u8"/extensions/VRMC_vrm/humanoid", u8"👤" },
      // { u8"/extensions/VRMC_vrm/*", u8"🌟" },
      //
      { u8"/extensions", u8"⭐" },
      // { u8"/extensions/*", u8"⭐" },
      { u8"/extras", u8"⭐" },
      // { u8"/extras/*", u8"⭐" },
      { u8"/extensionsUsed", u8"⭐" },
      { u8"/images", u8"🖼" },
      { u8"/images/*", u8"🖼" },
      { u8"/textures", u8"🖼" },
      { u8"/textures/*", u8"🖼" },
      { u8"/samplers", u8"🖼" },
      { u8"/samplers/*", u8"🖼" },
      { u8"/bufferViews", u8"🔢" },
      { u8"/bufferViews/*", u8"🔢" },
      { u8"/buffers", u8"🔢" },
      { u8"/buffers/*", u8"🔢" },
      { u8"/accessors", u8"🔢" },
      { u8"/accessors/*", u8"🔢" },
      { u8"/meshes", u8"🔺" },
      { u8"/meshes/*", u8"🔺" },
      { u8"/skins", u8"🔺" },
      { u8"/skins/*", u8"🔺" },
      { u8"/materials", u8"🎨" },
      { u8"/materials/*", u8"🎨" },
      { u8"/nodes", u8"✳ " },
      { u8"/nodes/*", u8"✳ " },
      { u8"/scenes", u8"✳ " },
      { u8"/scenes/*", u8"✳ " },
      { u8"/scene", u8"✳ " },
      { u8"/animations", u8"▶ " },
      { u8"/animations/*", u8"▶ " },
      { u8"/asset", u8"📄" },
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
  // auto path = gltfjson::JsonPath(jsonpath);
  std::u8string icon;
  if (auto found = m_iconMap.Match(jsonpath)) {
    icon = *found;
  }
  label.Key += gltfjson::from_u8(icon);
  label.Key += gltfjson::from_u8(GetLastName(jsonpath));
  auto object = item->Object();
  if (object && object->find(u8"name") != object->end()) {
    label.Value = "{";
    label.Value += gltfjson::from_u8((*object)[u8"name"]->U8String());
    label.Value += "}";
  } else {
    std::stringstream ss;
    ss << *item;
    label.Value = ss.str();
  }
  auto inserted =
    m_labelCache.insert({ { jsonpath.begin(), jsonpath.end() }, label });
  return inserted.first->second;
}
