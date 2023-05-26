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
