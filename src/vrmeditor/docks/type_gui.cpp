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

template<typename T>
static void
ShowGuiOptional(std::optional<T>& optional,
                const char* label,
                const char* label_new,
                const std::function<void(T&)> showGui)
{
  if (optional) {
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    bool visible = true;
    if (ImGui::CollapsingHeader(label, &visible)) {
      ImGui::PushID(&optional);
      ImGui::Indent();
      showGui(*optional);
      ImGui::Unindent();
      ImGui::PopID();
    }
    if (!visible) {
      optional = std::nullopt;
    }
  } else {
    if (ImGui::Button(label_new)) {
      optional = T{};
    }
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
        std::optional<uint32_t> index,
        gltfjson::format::ChildOfRootProperty& prop)
{
  ImGui::Text("%s/%d", base, index ? *index : -1);
  ShowGui("name", prop.Name);
}

// buffer/bufferView/accessor
void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Buffer& buffer)
{
  ShowGui("/buffers", root.Buffers.GetIndex(buffer), buffer);
}

void
ShowGui(const gltfjson::format::Root& root,
        gltfjson::format::BufferView& bufferView)
{
  ShowGui("/bufferViews", root.BufferViews.GetIndex(bufferView), bufferView);
}

void
ShowGui(const gltfjson::format::Root& root,
        gltfjson::format::Accessor& accessor)
{
  ShowGui("/accessors", root.Accessors.GetIndex(accessor), accessor);
}

// image/sampler/texture/material/mesh
void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Image& image)
{
  ShowGui("/images", root.Images.GetIndex(image), image);
}

void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Sampler& sampler)
{
  ShowGui("/samplers", root.Samplers.GetIndex(sampler), sampler);
  // grapho::imgui::EnumCombo(
  //   "magFilter", &sampler.MagFilter,
  //   gltfjson::format::TextureMagFilterCombo);
  // grapho::imgui::EnumCombo(
  //   "minFilter", &sampler.MinFilter,
  //   gltfjson::format::TextureMinFilterCombo);
  // grapho::imgui::EnumCombo(
  //   "wrapS", &sampler.WrapS, gltfjson::format::TextureWrapCombo);
  // grapho::imgui::EnumCombo(
  //   "wrapT", &sampler.WrapT, gltfjson::format::TextureWrapCombo);
}

void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Texture& texture)
{
  ShowGui("/textures", root.Textures.GetIndex(texture), texture);
}

template<typename T>
void
SelectId(const char* label, gltfjson::format::Id& id, const T& values)
{
  uint32_t selected = id ? *id : -1;
  using TUPLE = std::tuple<uint32_t, std::string>;
  std::vector<TUPLE> combo;
  for (int i = 0; i < values.Size(); ++i) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%d", i);
    combo.push_back({ i, buf });
  }
  std::span<const TUPLE> span(combo.data(), combo.size());

  grapho::imgui::GenericCombo<uint32_t>(label, &selected, span);
  if (selected >= 0 && selected < values.Size()) {
    id = selected;
  } else {
    id = std::nullopt;
  }
}

static void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::TextureInfo& info)
{
  SelectId("Index", info.Index, root.Textures);
  ImGui::InputScalar("TexCoord", ImGuiDataType_U8, &info.TexCoord);
}

static void
ShowGui(const gltfjson::format::Root& root,
        gltfjson::format::NormalTextureInfo& info)
{
  ShowGui(root, *static_cast<gltfjson::format::TextureInfo*>(&info));
}

static void
ShowGui(const gltfjson::format::Root& root,
        gltfjson::format::OcclusionTextureInfo& info)
{
  ShowGui(root, *static_cast<gltfjson::format::TextureInfo*>(&info));
}

static void
ShowGui(const gltfjson::format::Root& root,
        gltfjson::format::PbrMetallicRoughness& pbr)
{
  ImGui::ColorEdit4("BaseColorFactor", pbr.BaseColorFactor.data());
  ShowGuiOptional<gltfjson::format::TextureInfo>(
    pbr.BaseColorTexture,
    "BaseColorTexture",
    "BaseColorTexture +",
    [root](auto& info) { ::ShowGui(root, info); });
  ImGui::SliderFloat("MetallicFactor", &pbr.MetallicFactor, 0, 1);
  ImGui::SliderFloat("RoughnessFactor", &pbr.RoughnessFactor, 0, 1);
}

void
ShowGui(const gltfjson::format::Root& root,
        gltfjson::format::Material& material)
{
  ShowGui("/materials", root.Materials.GetIndex(material), material);

  ShowGuiOptional<gltfjson::format::PbrMetallicRoughness>(
    material.PbrMetallicRoughness,
    "PbrMetallicRoughness",
    "PbrMetallicRoughness +",
    [&root](auto& pbr) { ::ShowGui(root, pbr); });

  ShowGuiOptional<gltfjson::format::NormalTextureInfo>(
    material.NormalTexture,
    "NormalTexture",
    "NormalTexture +",
    [&root](auto& info) { ::ShowGui(root, info); });

  ShowGuiOptional<gltfjson::format::OcclusionTextureInfo>(
    material.OcclusionTexture,
    "OcclusionTexture",
    "OcclusionTexture +",
    [&root](auto& info) { ::ShowGui(root, info); });

  ShowGuiOptional<gltfjson::format::TextureInfo>(
    material.EmissiveTexture,
    "EmissiveTexture",
    "EmissiveTexture +",
    [&root](auto& info) { ::ShowGui(root, info); });

  ImGui::ColorEdit3("EmissiveFactor", material.EmissiveFactor.data());
  grapho::imgui::EnumCombo(
    "AlphaMode", &material.AlphaMode, gltfjson::format::AlphaModesCombo);
  ImGui::SliderFloat("AlphaCutoff", &material.AlphaCutoff, 0, 1);
  ImGui::Checkbox("DoubleSided", &material.DoubleSided);
}
void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Mesh& mesh)
{
  ShowGui("/meshes", root.Meshes.GetIndex(mesh), mesh);
}

// skin/node/scene/animation
void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Skin& skin)
{
  ShowGui("/skins", root.Skins.GetIndex(skin), skin);
}
void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Node& node)
{
  ShowGui("/nodes", root.Nodes.GetIndex(node), node);
}
void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Scene& scene)
{
  ShowGui("/scenes", root.Scenes.GetIndex(scene), scene);
}
void
ShowGui(const gltfjson::format::Root& root,
        gltfjson::format::Animation& animation)
{
  ShowGui("/animations", root.Animations.GetIndex(animation), animation);
}
