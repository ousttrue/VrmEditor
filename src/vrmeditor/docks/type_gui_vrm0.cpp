#include <GL/glew.h>

#include "type_gui_vrm0.h"
#include "im_widgets.h"

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::vrm0::VRM vrm)
{
  ShowGuiString("ExpoertVersion", vrm.m_json, u8"expoertVersion");
  ShowGuiString("SpecVersion", vrm.m_json, u8"specVersion");
}
