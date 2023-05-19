#include <GL/glew.h>

#include "app.h"
#include "glr/gl3renderer.h"
#include "type_gui.h"
#include "type_gui_imgui.h"
#include <grapho/gl3/texture.h>
#include <grapho/imgui/widgets.h>

// TODO: ID LISt
// * foreach
// * remove
// * add

template<typename T>
// requires T::Name
void
SelectId(const char* label, gltfjson::format::Id* id, const T& values)
{
  uint32_t selected = *id ? *(*id) : -1;
  using TUPLE = std::tuple<uint32_t, std::string>;
  std::vector<TUPLE> combo;
  for (int i = 0; i < values.Size(); ++i) {
    char buf[64];
    snprintf(
      buf, sizeof(buf), "[%d] %s", i, (const char*)values[i].Name.c_str());
    combo.push_back({ i, buf });
  }
  std::span<const TUPLE> span(combo.data(), combo.size());

  grapho::imgui::GenericCombo<uint32_t>(label, &selected, span);
  if (selected >= 0 && selected < values.Size()) {
    *id = selected;
  } else {
    *id = std::nullopt;
  }

  if (*id) {
    ImGui::SameLine();
    if (ImGui::Button("x")) {
      *id = std::nullopt;
    }
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
  ImGui::BeginDisabled(true);
  ShowGui("Uri", buffer.Uri);
  ImGui::InputScalar("ByteLength", ImGuiDataType_U32, &buffer.ByteLength);
  ImGui::EndDisabled();
}

void
ShowGui(const gltfjson::format::Root& root,
        gltfjson::format::BufferView& bufferView)
{
  ShowGui("/bufferViews", root.BufferViews.GetIndex(bufferView), bufferView);
  ImGui::BeginDisabled(true);
  SelectId("Buffer", &bufferView.Buffer, root.Buffers);
  ImGui::InputScalar("ByteOffset", ImGuiDataType_U32, &bufferView.ByteOffset);
  ImGui::InputScalar("ByteLength", ImGuiDataType_U32, &bufferView.ByteLength);
  ImGui::InputScalar("ByteStride", ImGuiDataType_U32, &bufferView.ByteStride);
  grapho::imgui::EnumCombo(
    "Target", &bufferView.Target, gltfjson::format::TargetsCombo);
  ImGui::EndDisabled();
}

void
ShowGui(const gltfjson::format::Root& root,
        gltfjson::format::Accessor& accessor)
{
  ShowGui("/accessors", root.Accessors.GetIndex(accessor), accessor);
  ImGui::BeginDisabled(true);
  SelectId("BufferView", &accessor.BufferView, root.BufferViews);
  ImGui::InputScalar("ByteOffset", ImGuiDataType_U32, &accessor.ByteOffset);
  grapho::imgui::EnumCombo("ComponentType",
                           &accessor.ComponentType,
                           gltfjson::format::ComponentTypesCombo);
  ImGui::Checkbox("Normalized", &accessor.Normalized);
  ImGui::InputScalar("Count", ImGuiDataType_U32, &accessor.Count);
  grapho::imgui::EnumCombo(
    "Type", &accessor.Type, gltfjson::format::TypesCombo);
  // std::vector<float> Max;
  // std::vector<float> Min;
  ShowGuiOptional<gltfjson::format::Sparse>(
    accessor.Sparse, "Sparse", "Sparse +", [](auto& sparse) {});
  ImGui::EndDisabled();
}

// image/sampler/texture/material/mesh
void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Image& image)
{
  ShowGui("/images", root.Images.GetIndex(image), image);
  ImGui::BeginDisabled(true);
  ShowGui("Uri", image.Uri);
  ShowGui("MimeType", image.MimeType);
  std::u8string MimeType;
  SelectId("BufferView", &image.BufferView, root.BufferViews);
  ImGui::EndDisabled();
}

void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Sampler& sampler)
{
  ShowGui("/samplers", root.Samplers.GetIndex(sampler), sampler);
  grapho::imgui::EnumCombo(
    "magFilter", &sampler.MagFilter, gltfjson::format::TextureMagFilterCombo);
  grapho::imgui::EnumCombo(
    "minFilter", &sampler.MinFilter, gltfjson::format::TextureMinFilterCombo);
  grapho::imgui::EnumCombo(
    "wrapS", &sampler.WrapS, gltfjson::format::TextureWrapCombo);
  grapho::imgui::EnumCombo(
    "wrapT", &sampler.WrapT, gltfjson::format::TextureWrapCombo);
}

void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Texture& texture)
{
  ShowGui("/textures", root.Textures.GetIndex(texture), texture);
  SelectId("Sampler", &texture.Sampler, root.Samplers);
  SelectId("Source", &texture.Source, root.Images);
}

static void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::TextureInfo& info)
{
  SelectId("Index", &info.Index, root.Textures);
  if (info.Index) {
    if (auto texture = App::Instance().GetTexture(
          root, *info.Index, libvrm::gltf::ColorSpace::Linear)) {
      ImGui::Image((ImTextureID)(int64_t)texture->Handle(), { 150, 150 });
    }
  } else {
    auto pos = ImGui::GetCursorPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRect(pos, { pos.x + 150, pos.y + 150 }, ImColor());
  }
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
    [&root](auto& info) { ::ShowGui(root, info); });
  ImGui::SliderFloat("MetallicFactor", &pbr.MetallicFactor, 0, 1);
  ImGui::SliderFloat("RoughnessFactor", &pbr.RoughnessFactor, 0, 1);
  ShowGuiOptional<gltfjson::format::TextureInfo>(
    pbr.MetallicRoughnessTexture,
    "MetallicRoughnessTexture",
    "MetallicRoughnessTexture +",
    [&root](auto& info) { ::ShowGui(root, info); });
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

static void
ShowGui(const gltfjson::format::Root& root,
        gltfjson::format::MeshPrimitiveAttributes& attribute)
{
  SelectId("COLOR_0", &attribute.COLOR_0, root.Accessors);
  SelectId("JOINTS_0", &attribute.JOINTS_0, root.Accessors);
  SelectId("NORMAL", &attribute.NORMAL, root.Accessors);
  SelectId("POSITION", &attribute.POSITION, root.Accessors);
  SelectId("TANGENT", &attribute.TANGENT, root.Accessors);
  SelectId("TEXCOORD_0", &attribute.TEXCOORD_0, root.Accessors);
  SelectId("TEXCOORD_1", &attribute.TEXCOORD_1, root.Accessors);
  SelectId("TEXCOORD_2", &attribute.TEXCOORD_2, root.Accessors);
  SelectId("TEXCOORD_3", &attribute.TEXCOORD_3, root.Accessors);
  SelectId("WEIGHTS_0", &attribute.WEIGHTS_0, root.Accessors);
}

static void
ShowGui(const gltfjson::format::Root& root,
        gltfjson::format::MeshPrimitive& prim)
{
  ImGui::PushID(&prim);
  ImGui::BeginDisabled(true);
  ShowGui(root, prim.Attributes);
  SelectId("Indices", &prim.Indices, root.Accessors);
  ImGui::EndDisabled();
  SelectId("Material", &prim.Material, root.Materials);
  ImGui::BeginDisabled(true);
  grapho::imgui::EnumCombo(
    "Mode", &prim.Mode, gltfjson::format::MeshPrimitiveTopologyCombo);
  // std::vector<MeshPrimitiveMorphTarget> Targets;
  ImGui::EndDisabled();
  ImGui::PopID();
}

void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Mesh& mesh)
{
  ShowGui("/meshes", root.Meshes.GetIndex(mesh), mesh);
  int i = 0;
  for (auto& prim : mesh.Primitives) {
    char buf[64];
    snprintf(buf, sizeof(buf), "[%d]", i++);
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader(buf)) {
      ImGui::Indent();
      ShowGui(root, prim);
      ImGui::Unindent();
    }
  }

  // TODO: morph weight
}

// skin/node/scene/animation
void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Skin& skin)
{
  ShowGui("/skins", root.Skins.GetIndex(skin), skin);
  ImGui::BeginDisabled(true);
  SelectId("InverseBindMatrices", &skin.InverseBindMatrices, root.Accessors);
  SelectId("Skeleton", &skin.Skeleton, root.Nodes);
  // std::vector<uint32_t> Joints;
  ImGui::EndDisabled();
}

void
ShowGui(const gltfjson::format::Root& root, gltfjson::format::Node& node)
{
  ShowGui("/nodes", root.Nodes.GetIndex(node), node);
  // Id Camera;
  // std::vector<uint32_t> Children;
  SelectId("Skin", &node.Skin, root.Nodes);
  ShowGuiOptional<std::array<float, 16>>(
    node.Matrix, "Matrix", "Matrix +", [](auto& m) {
      ImGui::InputFloat4("##m0", m.data());
      ImGui::InputFloat4("##m1", m.data() + 4);
      ImGui::InputFloat4("##m2", m.data() + 8);
      ImGui::InputFloat4("##m3", m.data() + 12);
    });
  SelectId("Mesh", &node.Mesh, root.Meshes);
  ShowGuiOptional<std::array<float, 4>>(
    node.Rotation, "Rotation", "Rotaton +", [](auto& r) {
      ImGui::InputFloat4("##rotation", r.data());
    });
  ShowGuiOptional<std::array<float, 3>>(
    node.Scale, "Scale", "Scale +", [](auto& s) {
      ImGui::InputFloat3("##scale", s.data());
    });
  ShowGuiOptional<std::array<float, 3>>(
    node.Translation, "Translation", "Translation +", [](auto& t) {
      ImGui::InputFloat3("##translation", t.data());
    });
  // std::vector<float> Weights;
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
