#include <GL/glew.h>

#include "im_widgets.h"
#include "type_gui_vrm0.h"

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::vrm0::VRM vrm)
{
  ShowGuiString("ExpoertVersion", vrm.m_json, u8"expoertVersion");
  ShowGuiString("SpecVersion", vrm.m_json, u8"specVersion");
}

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::vrm0::Meta meta)
{
  ShowGuiString("Title", meta.m_json, u8"title");
  ShowGuiString("Version", meta.m_json, u8"version");
  ShowGuiString("Author", meta.m_json, u8"author");
  ShowGuiString("ContactInformation", meta.m_json, u8"contactInformation");
  ShowGuiString("Reference", meta.m_json, u8"reference");
  SelectId("Texture", meta.m_json, u8"texture", root.Textures.m_json);
  ShowGuiString("AllowedUser", meta.m_json, u8"allowedUserName");
  ShowGuiString("ViolentUsage", meta.m_json, u8"violentUssageName");
  ShowGuiString("SexualUsage", meta.m_json, u8"sexualUssageName");
  ShowGuiString("CommercialUsage", meta.m_json, u8"commercialUssageName");
  ShowGuiString("OtherPermissionUrl", meta.m_json, u8"otherPermissionUrl");
  ShowGuiString("License", meta.m_json, u8"licenseName");
}

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::vrm0::Humanoid humanoid)
{
}

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::vrm0::FirstPerson firstPerson)
{
  // FirstPerson
  SelectId("FirstPersonBone",
           firstPerson.m_json,
           u8"firstPersonBone",
           root.Nodes.m_json);
  // LookAt
  ShowGuiString("LookAtType", firstPerson.m_json, u8"lookAtType");
}

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::vrm0::BlendShapeGroup blendShapeGroup)
{
  ShowGuiString("Name", blendShapeGroup.m_json, u8"name");
  // const auto Name() const { return m_string<u8"name">(); }
  // const auto Preset() const { return m_string<u8"presetName">(); }
  // JsonArray<MorphBind, u8"binds"> MorphBinds;
  // JsonArray<MaterialBind, u8"materialValues"> MaterialBinds;
  // const auto IsBinary() const { return m_ptr<bool, u8"isBinary">(); }
}

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::vrm0::Spring spring)
{
  ShowGuiString("Comment", spring.m_json, u8"comment");
  // auto Stifness() const { return m_ptr<float, u8"stiffiness">(); }
  // auto GravityPower() const { return m_ptr<float, u8"gravityPower">(); }
  // auto GravityDir() const
  // {
  //   return m_ptr<gltfjson::tree::ObjectValue, u8"gravityDir">();
  // }
  // auto DragForce() const { return m_ptr<float, u8"dragForce">(); }
  // auto Center() const { return m_id<u8"center">(); }
  // auto HitRadius() const { return m_ptr<float, u8"hitRadius">(); }
  // auto Bones() const { return m_ptr<gltfjson::tree::ArrayValue, u8"bones">();
  // } auto ColliderGroups() const
  // {
  //   return m_ptr<gltfjson::tree::ArrayValue, u8"colliderGroups">();
  // }
}

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::vrm0::ColliderGroup colliderGroup)
{
  SelectId("Node", colliderGroup.m_json, u8"node", root.Nodes.m_json);
}
