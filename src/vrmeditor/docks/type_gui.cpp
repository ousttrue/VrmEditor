#include <GL/glew.h>

#include "glr/gl3renderer.h"
#include "scene_gui_material.h"
#include "type_gui.h"
#include <grapho/gl3/texture.h>
#include <grapho/imgui/widgets.h>
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
  ShowGui("copyright", asset.Copyright);
  ShowGui("generator", asset.Generator);
  ShowGui("version", asset.Version);
  ShowGui("minversion", asset.MinVersion);
}

static void
ShowGui(const char* base,
        uint32_t index,
        gltfjson::format::ChildOfRootProperty& prop)
{
  ImGui::Text("%s/%d", base, index);
  ShowGui("name", prop.Name);
}

// buffer/bufferView/accessor
void
ShowGui(uint32_t index, gltfjson::format::Buffer& buffer)
{
  ShowGui("/buffers", index, buffer);
}

void
ShowGui(uint32_t index, gltfjson::format::BufferView& bufferView)
{
  ShowGui("/bufferViews", index, bufferView);
}

void
ShowGui(uint32_t index, gltfjson::format::Accessor& accessor)
{
  ShowGui("/accessors", index, accessor);
}

// image/sampler/texture/material/mesh
void
ShowGui(uint32_t index, gltfjson::format::Image& image)
{
  ShowGui("/images", index, image);
}

void
ShowGui(uint32_t index, gltfjson::format::Sampler& sampler)
{
  ShowGui("/samplers", index, sampler);
  // grapho::imgui::EnumCombo(
  //   "magFilter", &sampler.MagFilter, gltfjson::format::TextureMagFilterCombo);
  // grapho::imgui::EnumCombo(
  //   "minFilter", &sampler.MinFilter, gltfjson::format::TextureMinFilterCombo);
  // grapho::imgui::EnumCombo(
  //   "wrapS", &sampler.WrapS, gltfjson::format::TextureWrapCombo);
  // grapho::imgui::EnumCombo(
  //   "wrapT", &sampler.WrapT, gltfjson::format::TextureWrapCombo);
}

void
ShowGui(uint32_t index, gltfjson::format::Texture& texture)
{
  ShowGui("/textures", index, texture);
}

void
ShowGui(uint32_t index, gltfjson::format::Material& material)
{
  ShowGui("/materials", index, material);

  // std::optional<PbrMetallicRoughness> PbrMetallicRoughness;
  // std::optional<NormalTextureInfo> NormalTexture;
  // std::optional<OcclusionTextureInfo> OcclusionTexture;
  // std::optional<TextureInfo> EmissiveTexture;

  ImGui::ColorEdit3("EmissiveFactor", material.EmissiveFactor.data());
  grapho::imgui::EnumCombo(
    "AlphaMode", &material.AlphaMode, gltfjson::format::AlphaModesCombo);
  ImGui::SliderFloat("AlphaCutoff", &material.AlphaCutoff, 0, 1);
  ImGui::Checkbox("DoubleSided", &material.DoubleSided);
}
void
ShowGui(uint32_t index, gltfjson::format::Mesh& mesh)
{
  ShowGui("/meshes", index, mesh);
}

// skin/node/scene/animation
void
ShowGui(uint32_t index, gltfjson::format::Skin& skin)
{
  ShowGui("/skins", index, skin);
}
void
ShowGui(uint32_t index, gltfjson::format::Node& node)
{
  ShowGui("/nodes", index, node);
}
void
ShowGui(uint32_t index, gltfjson::format::Scene& scene)
{
  ShowGui("/scenes", index, scene);
}
void
ShowGui(uint32_t index, gltfjson::format::Animation& animation)
{
  ShowGui("/animations", index, animation);
}

