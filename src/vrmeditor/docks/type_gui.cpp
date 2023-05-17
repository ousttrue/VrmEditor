#include "type_gui.h"
#include <imgui.h>

namespace ImGui {

struct InputTextCallback_UserData
{
  std::u8string* Str;
  ImGuiInputTextCallback ChainCallback;
  void* ChainCallbackUserData;
};

static int
InputTextCallback(ImGuiInputTextCallbackData* data)
{
  InputTextCallback_UserData* user_data =
    (InputTextCallback_UserData*)data->UserData;
  if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
    // Resize string callback
    // If for some reason we refuse the new length (BufTextLen) and/or capacity
    // (BufSize) we need to set them back to what we want.
    auto str = user_data->Str;
    IM_ASSERT(data->Buf == (char*)str->c_str());
    str->resize(data->BufTextLen);
    data->Buf = (char*)str->c_str();
  } else if (user_data->ChainCallback) {
    // Forward to user callback, if any
    data->UserData = user_data->ChainCallbackUserData;
    return user_data->ChainCallback(data);
  }
  return 0;
}

static bool
InputText(const char* label,
          std::u8string* str,
          ImGuiInputTextFlags flags = 0,
          ImGuiInputTextCallback callback = 0,
          void* user_data = 0)
{
  IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
  flags |= ImGuiInputTextFlags_CallbackResize;

  InputTextCallback_UserData cb_user_data;
  cb_user_data.Str = str;
  cb_user_data.ChainCallback = callback;
  cb_user_data.ChainCallbackUserData = user_data;
  return InputText(label,
                   (char*)str->c_str(),
                   str->capacity() + 1,
                   flags,
                   InputTextCallback,
                   &cb_user_data);
}
}

static void
ShowGui(const char* label, std::u8string& str)
{
  ImGui::InputText(label, &str);
}

void
ShowGui(gltfjson::format::Asset& asset)
{
  if (ImGui::CollapsingHeader("Asset", ImGuiTreeNodeFlags_None)) {
    ShowGui("copyright", asset.Copyright);
    ShowGui("generator", asset.Generator);
    ShowGui("version", asset.Version);
    ShowGui("minversion", asset.MinVersion);
  }
}
