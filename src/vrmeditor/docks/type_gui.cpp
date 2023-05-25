#include <GL/glew.h>

#include "app.h"
#include "glr/gl3renderer.h"
#include "printfbuffer.h"
#include "type_gui.h"
#include "type_gui_accessor.h"
#include "type_gui_imgui.h"
#include <grapho/gl3/texture.h>
#include <grapho/imgui/widgets.h>
#include <misc/cpp/imgui_stdlib.h>
#include <sstream>
#include <unordered_map>

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
    std::u8string* str = user_data->Str;
    IM_ASSERT(data->Buf == (const char*)str->c_str());
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
InputU8Text(const char* label,
            std::u8string* str,
            ImGuiInputTextFlags flags = 0,
            ImGuiInputTextCallback callback = nullptr,
            void* user_data = nullptr)
{
  IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
  flags |= ImGuiInputTextFlags_CallbackResize;

  InputTextCallback_UserData cb_user_data;
  cb_user_data.Str = str;
  cb_user_data.ChainCallback = callback;
  cb_user_data.ChainCallbackUserData = user_data;
  return ImGui::InputText(label,
                          (char*)str->c_str(),
                          str->capacity() + 1,
                          flags,
                          InputTextCallback,
                          &cb_user_data);
}

static void
ShowGuiString(const char* label,
              const gltfjson::tree::NodePtr& parentNode,
              std::u8string_view key)
{
  if (!parentNode) {
    return;
  }

  auto node = parentNode->Get(key);
  if (!node) {
    node = parentNode->Add(key, u8"");
  }
  auto p = node->Ptr<std::u8string>();
  if (!p) {
    node->Var = u8"";
    p = node->Ptr<std::u8string>();
  }
  InputU8Text(label, p);
}

static void
ShowGuiBool(const char* label,
            const gltfjson::tree::NodePtr& parentNode,
            std::u8string_view key)
{
  if (!parentNode) {
    return;
  }
  auto node = parentNode->Get(key);
  if (!node) {
    node = parentNode->Add(key, u8"");
  }

  auto p = node->Ptr<bool>();
  if (!p) {
    node->Var = false;
    p = node->Ptr<bool>();
  }

  ImGui::Checkbox(label, p);
}

static void
ShowGuiUInt32(const char* label,
              const gltfjson::tree::NodePtr& parentNode,
              std::u8string_view key)
{
  if (!parentNode) {
    return;
  }
  auto node = parentNode->Get(key);
  if (!node) {
    node = parentNode->Add(key, u8"");
  }

  auto p = node->Ptr<float>();
  if (!p) {
    node->Var = 0.0f;
    p = node->Ptr<float>();
  }

  auto value = (uint32_t)*p;
  if (ImGui::InputScalar(label, ImGuiDataType_U32, &value)) {
    *p = (float)value;
  }
}

template<typename T>
static void
ShowGuiEnum(const char* label,
            const gltfjson::tree::NodePtr& parentNode,
            std::u8string_view key,
            std::span<const std::tuple<T, const char*>> combo)
{
  if (!parentNode) {
    return;
  }
  auto node = parentNode->Get(key);
  if (!node) {
    node = parentNode->Add(key, u8"");
  }

  auto p = node->Ptr<float>();
  if (!p) {
    node->Var = 0.0f;
    p = node->Ptr<float>();
  }

  auto value = (T)*p;
  if (grapho::imgui::EnumCombo(label, &value, combo)) {
    *p = (float)value;
  }
}

static void
ShowGuiStringEnum(const char* label,
                  const gltfjson::tree::NodePtr& parentNode,
                  std::u8string_view key,
                  std::span<const std::tuple<int, std::string>> combo)
{
  if (!parentNode) {
    return;
  }
  auto node = parentNode->Get(key);
  if (!node) {
    node = parentNode->Add(key, u8"");
  }

  auto p = node->Ptr<std::u8string>();
  if (!p) {
    node->Var = u8"";
    p = node->Ptr<std::u8string>();
  }

  int i = 0;
  for (; i < combo.size(); ++i) {
    auto& str = std::get<1>(combo[i]);
    if (*p == gltfjson::tree::to_u8(str)) {
      break;
    }
  }

  if (grapho::imgui::GenericCombo(label, &i, combo)) {
    *p = gltfjson::tree::to_u8(std::get<1>(combo[i]));
  }
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a
// merged icon fonts (see docs/FONTS.md)
static void
HelpMarker(const char* desc)
{
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) &&
      ImGui::BeginTooltip()) {
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(desc);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}

void
SelectId(const char* label,
         const gltfjson::tree::NodePtr& idParent,
         const char8_t* key,
         const gltfjson::tree::NodePtr& arrayNode)
{
  if (!arrayNode) {
    return;
  }
  auto values = arrayNode->Array();
  if (!values) {
    return;
  }
  if (!idParent) {
    return;
  }

  float* id = nullptr;
  auto idNode = idParent->Get(key);
  if (idNode) {
    id = idNode->Ptr<float>();
  }

  auto selected = (uint32_t)(id ? *id : -1);
  using TUPLE = std::tuple<uint32_t, std::string>;
  std::vector<TUPLE> combo;
  PrintfBuffer buf;
  for (int i = 0; i < values->size(); ++i) {
    auto item = (*values)[i];
    combo.push_back(
      { i,
        buf.Printf(
          "%s[%d] %s", label, i, (const char*)item->U8String().c_str()) });
  }
  std::span<const TUPLE> span(combo.data(), combo.size());

  bool removed = false;

  grapho::imgui::GenericCombo<uint32_t>(label, &selected, span);
  if (selected >= 0 && selected < values->size()) {
    *id = (float)selected;
  } else {
    idParent->Remove(key);
  }

  if (id) {
    ImGui::SameLine();
    if (ImGui::Button("x")) {
      idParent->Remove(key);
    }
  }
}

template<typename T>
void
ListId(const char* label, std::vector<uint32_t>& list, const T& values)
{
  ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
  if (ImGui::CollapsingHeader(label)) {
    ImGui::Indent();
    std::stringstream ss;
    uint32_t i = 0;
    PrintfBuffer buf;
    for (auto it = list.begin(); it != list.end(); ++i) {
      ImGui::TextUnformatted(buf.Printf("%s[%d] %s",
                                        values.Name.c_str(),
                                        i,
                                        (const char*)values[*it].Name.c_str()));
      ImGui::SameLine();

      if (ImGui::Button(buf.Printf("x##%d", i))) {
        it = list.erase(it);
      } else {
        ++it;
      }
    }

    std::optional<uint32_t> new_value;
    // SelectId("+", &new_value, values);
    if (new_value) {
      list.push_back(*new_value);
    }
    ImGui::Unindent();
  }
}

template<typename T>
static void
ShowGuiVector(const char* label,
              std::vector<T>& vector,
              const std::function<void(size_t i, T&)>& showGui)
{
  if (ImGui::CollapsingHeader(label)) {
    ImGui::Indent();
    ImGui::PushID(&vector);
    PrintfBuffer buf;
    size_t i = 0;
    for (auto it = vector.begin(); it != vector.end(); ++i) {
      showGui(i, *it);
      ImGui::SameLine();
      if (ImGui::Button(buf.Printf("x##%s_%d", label, i))) {
        it = vector.erase(it);
      } else {
        ++it;
      }
    }

    if (ImGui::Button(buf.Printf("+##%s_%d", label, i))) {
      vector.push_back({});
    }
    ImGui::PopID();
    ImGui::Unindent();
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

// static void
// ShowGui(const char* label, std::u8string& str)
// {
//   ImGui::InputText(label, &str);
// }

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::Asset asset)
{
  ShowGuiString("copyright", asset.m_json, u8"copyright");
  ShowGuiString("generator", asset.m_json, u8"generator");
  ShowGuiString("version", asset.m_json, u8"version");
  ShowGuiString("minversion", asset.m_json, u8"minVersion");
}

static void
ShowGuiChildOfRoot(gltfjson::typing::ChildOfRootProperty& prop)
{
  ShowGuiString("name", prop.m_json, u8"name");
}

// buffer/bufferView/accessor
void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::Buffer buffer)
{
  ShowGuiChildOfRoot(buffer);
  ImGui::BeginDisabled(true);
  ShowGuiString("Uri", buffer.m_json, u8"uri");
  ImGui::InputScalar("ByteLength", ImGuiDataType_U32, buffer.ByteLength());
  ImGui::EndDisabled();
}

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::BufferView bufferView)
{
  ShowGuiChildOfRoot(bufferView);
  ImGui::BeginDisabled(true);

  SelectId(
    "Buffer", bufferView.m_json, u8"buffer", root.m_json->Get(u8"buffers"));
  ShowGuiUInt32("ByteOffset", bufferView.m_json, u8"byteOffset");
  ShowGuiUInt32("ByteLength", bufferView.m_json, u8"byteLength");
  ShowGuiUInt32("ByteStride", bufferView.m_json, u8"byteStride");
  ShowGuiEnum<gltfjson::format::Targets>(
    "Target", bufferView.m_json, u8"target", gltfjson::format::TargetsCombo);
  ImGui::EndDisabled();
}

namespace std {
template<>
class hash<
  std::tuple<gltfjson::format::ComponentTypes, gltfjson::format::Types>>
{
public:
  size_t operator()(const std::tuple<gltfjson::format::ComponentTypes,
                                     gltfjson::format::Types>& x) const
  {
    return hash<int>()((int)std::get<0>(x)) ^ hash<int>()((int)std::get<1>(x));
  }
};
}

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::Accessor accessor)
{
  ShowGuiChildOfRoot(accessor);
  ImGui::BeginDisabled(true);
  SelectId(
    "BufferView", accessor.m_json, u8"bufferView", root.BufferViews.m_json);
  auto value = (uint32_t)*accessor.ByteOffset();
  if (ImGui::InputScalar("ByteOffset", ImGuiDataType_U32, &value)) {
    *accessor.ByteOffset() = (float)value;
  }

  ShowGuiEnum<gltfjson::format::ComponentTypes>(
    "ComponentType",
    accessor.m_json,
    u8"componentType",
    gltfjson::format::ComponentTypesCombo);
  ShowGuiBool("Normalized", accessor.m_json, u8"normalized");
  ShowGuiUInt32("Count", accessor.m_json, u8"count");
  ShowGuiStringEnum(
    "Type",
    accessor.m_json,
    u8"type",
    { (const std::tuple<int, std::string>*)gltfjson::format::TypesCombo,
      std::size(gltfjson::format::TypesCombo) });

  ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
  // ShowGuiVector<float>("Max", accessor.Max, [](size_t i, auto& v) {
  //   if (i) {
  //     ImGui::SameLine();
  //   }
  //   ImGui::Text("%f", v);
  // });
  ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
  // ShowGuiVector<float>("Min", accessor.Min, [](size_t i, auto& v) {
  //   if (i) {
  //     ImGui::SameLine();
  //   }
  //   ImGui::Text("%f", v);
  // });
  // ShowGuiOptional<gltfjson::annotation::Sparse>(
  //   accessor.Sparse, "Sparse", "Sparse +", [](auto& sparse) {});
  ImGui::EndDisabled();

  using ShowAccessorTable =
    std::function<void(const gltfjson::typing::Root& root,
                       const gltfjson::typing::Bin& bin,
                       const gltfjson::typing::Accessor& accessor)>;

  using AccessorTypeKey =
    std::tuple<gltfjson::format::ComponentTypes, gltfjson::format::Types>;
  static std::unordered_map<AccessorTypeKey, ShowAccessorTable> s_GuiMap = {
    { { gltfjson::format::ComponentTypes::UNSIGNED_BYTE,
        gltfjson::format::Types::SCALAR },
      [](auto& root, auto& bin, auto& accessor) {
        if (auto values =
              bin.template GetAccessorBytes<uint8_t>(root, accessor)) {
          ShowGuiAccessorScalar<uint8_t>(*values);
        }
      } },
    { { gltfjson::format::ComponentTypes::UNSIGNED_SHORT,
        gltfjson::format::Types::SCALAR },
      [](auto& root, auto& bin, auto& accessor) {
        if (auto values =
              bin.template GetAccessorBytes<uint16_t>(root, accessor)) {
          ShowGuiAccessorScalar<uint16_t>(*values);
        }
      } },
    { { gltfjson::format::ComponentTypes::UNSIGNED_SHORT,
        gltfjson::format::Types::VEC4 },
      [](auto& root, auto& bin, auto& accessor) {
        if (auto values = bin.template GetAccessorBytes<runtimescene::ushort4>(
              root, accessor)) {
          ShowGuiAccessorInt4<runtimescene::ushort4>(*values);
        }
      } },
    { { gltfjson::format::ComponentTypes::UNSIGNED_INT,
        gltfjson::format::Types::SCALAR },
      [](auto& root, auto& bin, auto& accessor) {
        if (auto values =
              bin.template GetAccessorBytes<uint32_t>(root, accessor)) {
          ShowGuiAccessorScalar<uint32_t>(*values);
        }
      } },
    { { gltfjson::format::ComponentTypes::FLOAT,
        gltfjson::format::Types::VEC2 },
      [](auto& root, auto& bin, auto& accessor) {
        if (auto values = bin.template GetAccessorBytes<DirectX::XMFLOAT2>(
              root, accessor)) {
          ShowGuiAccessorVec2<DirectX::XMFLOAT2>(*values);
        }
      } },
    { { gltfjson::format::ComponentTypes::FLOAT,
        gltfjson::format::Types::VEC3 },
      [](auto& root, auto& bin, auto& accessor) {
        if (auto values = bin.template GetAccessorBytes<DirectX::XMFLOAT3>(
              root, accessor)) {
          ShowGuiAccessorVec3<DirectX::XMFLOAT3>(*values);
        }
      } },
    { { gltfjson::format::ComponentTypes::FLOAT,
        gltfjson::format::Types::VEC4 },
      [](auto& root, auto& bin, auto& accessor) {
        if (auto values = bin.template GetAccessorBytes<DirectX::XMFLOAT4>(
              root, accessor)) {
          ShowGuiAccessorVec4<DirectX::XMFLOAT4>(*values);
        }
      } },
    { { gltfjson::format::ComponentTypes::FLOAT,
        gltfjson::format::Types::MAT4 },
      [](auto& root, auto& bin, auto& accessor) {
        if (auto values = bin.template GetAccessorBytes<DirectX::XMFLOAT4X4>(
              root, accessor)) {
          ShowGuiAccessorMat4(*values);
        }
      } },
  };

  AccessorTypeKey key{
    (gltfjson::format::ComponentTypes)*accessor.ComponentType(),
    *gltfjson::format::types_from_str(accessor.Type())
  };
  auto found = s_GuiMap.find(key);
  if (found != s_GuiMap.end()) {
    found->second(root, bin, accessor);
  }
}

// image/sampler/texture/material/mesh
void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::Image image)
{
  ShowGuiChildOfRoot(image);
  ImGui::BeginDisabled(true);
  ShowGuiString("Uri", image.m_json, u8"uri");
  ShowGuiString("MimeType", image.m_json, u8"mimeType");
  std::u8string MimeType;
  SelectId("BufferView", image.m_json, u8"bufferView", root.BufferViews.m_json);
  ImGui::EndDisabled();
}

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::Sampler sampler)
{
  ShowGuiChildOfRoot(sampler);
  ShowGuiEnum<gltfjson::format::TextureMagFilter>(
    "magFilter",
    sampler.m_json,
    u8"magFilter",
    gltfjson::format::TextureMagFilterCombo);
  ShowGuiEnum<gltfjson::format::TextureMinFilter>(
    "minFilter",
    sampler.m_json,
    u8"minFilter",
    gltfjson::format::TextureMinFilterCombo);
  ShowGuiEnum<gltfjson::format::TextureWrap>(
    "wrapS", sampler.m_json, u8"wrapS", gltfjson::format::TextureWrapCombo);
  ShowGuiEnum<gltfjson::format::TextureWrap>(
    "wrapT", sampler.m_json, u8"wrapT", gltfjson::format::TextureWrapCombo);
}

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::Texture texture)
{
  ShowGuiChildOfRoot(texture);
  SelectId("Sampler", texture.m_json, u8"sampler", root.Samplers.m_json);
  SelectId("Source", texture.m_json, u8"source", root.Images.m_json);
}

static void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::TextureInfo& info)
{
  SelectId("Index", info.m_json, u8"index", root.Textures.m_json);
  if (auto texture = glr::GetOrCreateTexture(
        root, bin, *info.Index(), libvrm::gltf::ColorSpace::Linear)) {
    ImGui::Image((ImTextureID)(int64_t)texture->Handle(), { 150, 150 });
  } else {
    auto pos = ImGui::GetCursorPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRect(pos, { pos.x + 150, pos.y + 150 }, ImColor());
  }
  auto value = (uint32_t)*info.TexCoord();
  if (ImGui::InputScalar("TexCoord", ImGuiDataType_U8, &value)) {
    *info.TexCoord() = (float)value;
  }
}

static void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::NormalTextureInfo& info)
{
  ShowGui(root, bin, *static_cast<gltfjson::typing::TextureInfo*>(&info));
}

static void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::OcclusionTextureInfo& info)
{
  ShowGui(root, bin, *static_cast<gltfjson::typing::TextureInfo*>(&info));
}

static void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::PbrMetallicRoughness& pbr)
{
  // ImGui::ColorEdit4("BaseColorFactor", pbr.BaseColorFactor.data());
  // ShowGuiOptional<gltfjson::annotation::TextureInfo>(
  //   pbr.BaseColorTexture,
  //   "BaseColorTexture",
  //   "BaseColorTexture +",
  //   [&root, &bin](auto& info) { ::ShowGui(root, bin, info); });
  // ImGui::SliderFloat("MetallicFactor", &pbr.MetallicFactor, 0, 1);
  // ImGui::SliderFloat("RoughnessFactor", &pbr.RoughnessFactor, 0, 1);

  HelpMarker(
    "The metallic-roughness texture. The metalness values are sampled from "
    "the B channel. The roughness values are sampled from the G channel. "
    "These values **MUST** be encoded with a linear transfer function. If "
    "other channels are present (R or A), they **MUST** be ignored for "
    "metallic-roughness calculations. When undefined, the texture **MUST** "
    "be sampled as having `1.0` in G and B components.");
  // ShowGuiOptional<gltfjson::annotation::TextureInfo>(
  //   pbr.MetallicRoughnessTexture,
  //   "MetallicRoughnessTexture",
  //   "MetallicRoughnessTexture +",
  //   [&root, &bin](auto& info) { ::ShowGui(root, bin, info); });
}

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::Material material)
{
  ShowGuiChildOfRoot(material);

  // ShowGuiOptional<gltfjson::annotation::PbrMetallicRoughness>(
  //   material.PbrMetallicRoughness,
  //   "PbrMetallicRoughness",
  //   "PbrMetallicRoughness +",
  //   [&root, &bin](auto& pbr) { ::ShowGui(root, bin, pbr); });
  //
  // ShowGuiOptional<gltfjson::annotation::NormalTextureInfo>(
  //   material.NormalTexture,
  //   "NormalTexture",
  //   "NormalTexture +",
  //   [&root, &bin](auto& info) { ::ShowGui(root, bin, info); });

  HelpMarker("The occlusion texture. The occlusion values are linearly sampled "
             "from the R channel. Higher values indicate areas that receive "
             "full indirect lighting and lower values indicate no indirect "
             "lighting. If other channels are present (GBA), they **MUST** be "
             "ignored for occlusion calculations. When undefined, the material "
             "does not have an occlusion texture.");
  // ShowGuiOptional<gltfjson::annotation::OcclusionTextureInfo>(
  //   material.OcclusionTexture,
  //   "OcclusionTexture",
  //   "OcclusionTexture +",
  //   [&root, &bin](auto& info) { ::ShowGui(root, bin, info); });
  //
  // ShowGuiOptional<gltfjson::annotation::TextureInfo>(
  //   material.EmissiveTexture,
  //   "EmissiveTexture",
  //   "EmissiveTexture +",
  //   [&root, &bin](auto& info) { ::ShowGui(root, bin, info); });
  //
  // ImGui::ColorEdit3("EmissiveFactor", material.EmissiveFactor.data());
  ShowGuiEnum<gltfjson::format::AlphaModes>("AlphaMode",
                                            material.m_json,
                                            u8"alphaMode",
                                            gltfjson::format::AlphaModesCombo);
  // ImGui::SliderFloat("AlphaCutoff", &material.AlphaCutoff, 0, 1);
  ShowGuiBool("DoubleSided", material.m_json, u8"doubleSided");
}

static void
ShowGui(const gltfjson::typing::Root& root,
        gltfjson::typing::MeshPrimitiveAttributes attribute)
{
  SelectId("POSITION", attribute.m_json, u8"POSITION", root.Accessors.m_json);
  SelectId("NORMAL", attribute.m_json, u8"NORMAL", root.Accessors.m_json);
  SelectId(
    "TEXCOORD_0", attribute.m_json, u8"TEXCOORD_0", root.Accessors.m_json);
  SelectId(
    "TEXCOORD_1", attribute.m_json, u8"TEXCOORD_1", root.Accessors.m_json);
  SelectId(
    "TEXCOORD_2", attribute.m_json, u8"TEXCOORD_2", root.Accessors.m_json);
  SelectId(
    "TEXCOORD_3", attribute.m_json, u8"TEXCOORD_3", root.Accessors.m_json);
  SelectId("COLOR_0", attribute.m_json, u8"COLOR_0", root.Accessors.m_json);
  SelectId("TANGENT", attribute.m_json, u8"TANGENT", root.Accessors.m_json);
  SelectId("JOINTS_0", attribute.m_json, u8"JOINTS_0", root.Accessors.m_json);
  SelectId("WEIGHTS_0", attribute.m_json, u8"WEIGHTS_0", root.Accessors.m_json);
}

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::MeshPrimitiveMorphTarget target)
{
  SelectId("POSITION", target.m_json, u8"POSITION", root.Accessors.m_json);
  SelectId("NORMAL", target.m_json, u8"NORMAL", root.Accessors.m_json);
}

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::MeshPrimitive prim)
{
  ImGui::PushID(&prim);
  {
    ImGui::BeginDisabled(true);
    ShowGui(root, *prim.Attributes());
    SelectId("Indices", prim.m_json, u8"indices", root.Accessors.m_json);
    ImGui::EndDisabled();
  }
  SelectId("Material", prim.m_json, u8"material", root.Materials.m_json);
  {
    ImGui::BeginDisabled(true);
    ShowGuiEnum<gltfjson::format::MeshPrimitiveTopology>(
      "Mode",
      prim.m_json,
      u8"mode",
      gltfjson::format::MeshPrimitiveTopologyCombo);
    ImGui::EndDisabled();
  }
  ImGui::PopID();
}

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::Mesh mesh)
{
  ShowGuiChildOfRoot(mesh);

  PrintfBuffer buf;
  // int i = 0;
  // for (auto& prim : mesh.Primitives) {
  //
  //   ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
  //   if (ImGui::CollapsingHeader(buf.Printf("Primitives.%d", i++))) {
  //     ImGui::Indent();
  //     ShowGui(root, prim);
  //     ImGui::Unindent();
  //   }
  // }

  // ShowGuiVector<float>("Weights", mesh.Weights, [&buf](size_t i, auto& v) {
  //   ImGui::SliderFloat(buf.Printf("##Weights_%zu", i), &v, 0, 1);
  // });
}

// skin/node/scene/animation
void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::Skin skin)
{
  ShowGuiChildOfRoot(skin);
  ImGui::BeginDisabled(true);
  SelectId("InverseBindMatrices",
           skin.m_json,
           u8"inverseBindMatrices",
           root.Accessors.m_json);
  SelectId("Skeleton", skin.m_json, u8"skeleton", root.Nodes.m_json);
  // ListId("Joints", skin.Joints, root.Nodes);
  ImGui::EndDisabled();
}

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::Node node)
{
  ShowGuiChildOfRoot(node);
  // Id Camera;
  // ListId("Children", node.Children, root.Nodes);
  SelectId("Skin", node.m_json, u8"skin", root.Nodes.m_json);
  // ShowGuiOptional<std::array<float, 16>>(
  //   node.Matrix, "Matrix", "Matrix +", [](auto& m) {
  //     ImGui::InputFloat4("##m0", m.data());
  //     ImGui::InputFloat4("##m1", m.data() + 4);
  //     ImGui::InputFloat4("##m2", m.data() + 8);
  //     ImGui::InputFloat4("##m3", m.data() + 12);
  //   });
  // // SelectId("Mesh", &node.Mesh, root.Meshes);
  // ShowGuiOptional<std::array<float, 4>>(
  //   node.Rotation, "Rotation", "Rotaton +", [](auto& r) {
  //     ImGui::InputFloat4("##rotation", r.data());
  //   });
  // ShowGuiOptional<std::array<float, 3>>(
  //   node.Scale, "Scale", "Scale +", [](auto& s) {
  //     ImGui::InputFloat3("##scale", s.data());
  //   });
  // ShowGuiOptional<std::array<float, 3>>(
  //   node.Translation, "Translation", "Translation +", [](auto& t) {
  //     ImGui::InputFloat3("##translation", t.data());
  //   });
  //
  // PrintfBuffer buf;
  // ShowGuiVector<float>("Weights", node.Weights, [&buf](size_t i, auto& value)
  // {
  //   ImGui::SliderFloat(buf.Printf("##Weights_%zu", i), &value, 0, 1);
  // });
}

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::Scene scene)
{
  ShowGuiChildOfRoot(scene);
  // ListId("Nodes", scene.Nodes, root.Nodes);
}

void
ShowGui(const gltfjson::typing::Root& root,
        const gltfjson::typing::Bin& bin,
        gltfjson::typing::Animation animation)
{
  ShowGuiChildOfRoot(animation);
}
