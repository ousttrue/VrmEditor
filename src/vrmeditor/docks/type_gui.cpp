#include <GL/glew.h>

#include "app.h"
#include "glr/gl3renderer.h"
#include "type_gui.h"
#include "type_gui_accessor.h"
#include "type_gui_imgui.h"
#include <grapho/gl3/texture.h>
#include <grapho/imgui/widgets.h>
#include <misc/cpp/imgui_stdlib.h>
#include <sstream>
#include <unordered_map>

class PrintfBuffer
{
  char m_buf[256];

public:
  const char* Printf(const char* fmt, ...)
  {
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    vsnprintf(m_buf, sizeof(m_buf), fmt, arg_ptr);
    va_end(arg_ptr);
    return m_buf;
  }
};

template<typename T>
// requires T::Name
void
SelectId(const char* label, gltfjson::format::Id* id, const T& values)
{
  uint32_t selected = *id ? *(*id) : -1;
  using TUPLE = std::tuple<uint32_t, std::string>;
  std::vector<TUPLE> combo;
  PrintfBuffer buf;
  for (int i = 0; i < values.Size(); ++i) {
    combo.push_back({ i,
                      buf.Printf("%s[%d] %s",
                                 values.Name.c_str(),
                                 i,
                                 (const char*)values[i].Name.c_str()) });
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
    SelectId("+", &new_value, values);
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
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Buffer& buffer)
{
  ShowGui("/buffers", root.Buffers.GetIndex(buffer), buffer);
  ImGui::BeginDisabled(true);
  ShowGui("Uri", buffer.Uri);
  ImGui::InputScalar("ByteLength", ImGuiDataType_U32, &buffer.ByteLength);
  ImGui::EndDisabled();
}

void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
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
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
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

  ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
  ShowGuiVector<float>("Max", accessor.Max, [](size_t i, auto& v) {
    if (i) {
      ImGui::SameLine();
    }
    ImGui::Text("%f", v);
  });
  ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
  ShowGuiVector<float>("Min", accessor.Min, [](size_t i, auto& v) {
    if (i) {
      ImGui::SameLine();
    }
    ImGui::Text("%f", v);
  });
  ShowGuiOptional<gltfjson::format::Sparse>(
    accessor.Sparse, "Sparse", "Sparse +", [](auto& sparse) {});
  ImGui::EndDisabled();

  using ShowAccessorTable =
    std::function<void(const gltfjson::format::Root& root,
                       const gltfjson::format::Bin& bin,
                       const gltfjson::format::Accessor& accessor)>;
  static std::unordered_map<
    std::tuple<gltfjson::format::ComponentTypes, gltfjson::format::Types>,
    ShowAccessorTable>
    s_GuiMap = {
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
          if (auto values =
                bin.template GetAccessorBytes<runtimescene::ushort4>(
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

  auto found = s_GuiMap.find({ accessor.ComponentType, accessor.Type });
  if (found != s_GuiMap.end()) {
    found->second(root, bin, accessor);
  }
}

// image/sampler/texture/material/mesh
void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Image& image)
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
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Sampler& sampler)
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
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Texture& texture)
{
  ShowGui("/textures", root.Textures.GetIndex(texture), texture);
  SelectId("Sampler", &texture.Sampler, root.Samplers);
  SelectId("Source", &texture.Source, root.Images);
}

static void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::TextureInfo& info)
{
  SelectId("Index", &info.Index, root.Textures);
  if (auto texture = glr::GetOrCreateTexture(
        root, bin, info.Index, libvrm::gltf::ColorSpace::Linear)) {
    ImGui::Image((ImTextureID)(int64_t)texture->Handle(), { 150, 150 });
  } else {
    auto pos = ImGui::GetCursorPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRect(pos, { pos.x + 150, pos.y + 150 }, ImColor());
  }
  ImGui::InputScalar("TexCoord", ImGuiDataType_U8, &info.TexCoord);
}

static void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::NormalTextureInfo& info)
{
  ShowGui(root, bin, *static_cast<gltfjson::format::TextureInfo*>(&info));
}

static void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::OcclusionTextureInfo& info)
{
  ShowGui(root, bin, *static_cast<gltfjson::format::TextureInfo*>(&info));
}

static void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::PbrMetallicRoughness& pbr)
{
  ImGui::ColorEdit4("BaseColorFactor", pbr.BaseColorFactor.data());
  ShowGuiOptional<gltfjson::format::TextureInfo>(
    pbr.BaseColorTexture,
    "BaseColorTexture",
    "BaseColorTexture +",
    [&root, &bin](auto& info) { ::ShowGui(root, bin, info); });
  ImGui::SliderFloat("MetallicFactor", &pbr.MetallicFactor, 0, 1);
  ImGui::SliderFloat("RoughnessFactor", &pbr.RoughnessFactor, 0, 1);
  ShowGuiOptional<gltfjson::format::TextureInfo>(
    pbr.MetallicRoughnessTexture,
    "MetallicRoughnessTexture",
    "MetallicRoughnessTexture +",
    [&root, &bin](auto& info) { ::ShowGui(root, bin, info); });
}

void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Material& material)
{
  ShowGui("/materials", root.Materials.GetIndex(material), material);

  ShowGuiOptional<gltfjson::format::PbrMetallicRoughness>(
    material.PbrMetallicRoughness,
    "PbrMetallicRoughness",
    "PbrMetallicRoughness +",
    [&root, &bin](auto& pbr) { ::ShowGui(root, bin, pbr); });

  ShowGuiOptional<gltfjson::format::NormalTextureInfo>(
    material.NormalTexture,
    "NormalTexture",
    "NormalTexture +",
    [&root, &bin](auto& info) { ::ShowGui(root, bin, info); });

  ShowGuiOptional<gltfjson::format::OcclusionTextureInfo>(
    material.OcclusionTexture,
    "OcclusionTexture",
    "OcclusionTexture +",
    [&root, &bin](auto& info) { ::ShowGui(root, bin, info); });

  ShowGuiOptional<gltfjson::format::TextureInfo>(
    material.EmissiveTexture,
    "EmissiveTexture",
    "EmissiveTexture +",
    [&root, &bin](auto& info) { ::ShowGui(root, bin, info); });

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
  SelectId("POSITION", &attribute.POSITION, root.Accessors);
  SelectId("NORMAL", &attribute.NORMAL, root.Accessors);
  SelectId("TEXCOORD_0", &attribute.TEXCOORD_0, root.Accessors);
  SelectId("TEXCOORD_1", &attribute.TEXCOORD_1, root.Accessors);
  SelectId("TEXCOORD_2", &attribute.TEXCOORD_2, root.Accessors);
  SelectId("TEXCOORD_3", &attribute.TEXCOORD_3, root.Accessors);
  SelectId("COLOR_0", &attribute.COLOR_0, root.Accessors);
  SelectId("TANGENT", &attribute.TANGENT, root.Accessors);
  SelectId("JOINTS_0", &attribute.JOINTS_0, root.Accessors);
  SelectId("WEIGHTS_0", &attribute.WEIGHTS_0, root.Accessors);
}

static void
ShowGui(const char* label,
        const gltfjson::format::Root& root,
        gltfjson::format::MeshPrimitiveMorphTarget& target)
{
  if (ImGui::CollapsingHeader(label)) {
    ImGui::PushID(&target);
    SelectId("NORMAL", &target.NORMAL, root.Accessors);
    SelectId("POSITION", &target.POSITION, root.Accessors);
    ImGui::PopID();
  }
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
  ImGui::EndDisabled();

  PrintfBuffer buf;
  // ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
  if (ImGui::CollapsingHeader(
        buf.Printf("Targets(%zu)", prim.Targets.size()))) {
    ImGui::Indent();
    ImGui::BeginDisabled(true);
    int i = 0;
    for (auto& target : prim.Targets) {
      ShowGui(buf.Printf("[%d]", i++), root, target);
    }
    ImGui::EndDisabled();
    ImGui::Unindent();
  }

  ImGui::PopID();
}

void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Mesh& mesh)
{
  ShowGui("/meshes", root.Meshes.GetIndex(mesh), mesh);
  int i = 0;

  PrintfBuffer buf;
  for (auto& prim : mesh.Primitives) {

    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader(buf.Printf("Primitives.%d", i++))) {
      ImGui::Indent();
      ShowGui(root, prim);
      ImGui::Unindent();
    }
  }

  ShowGuiVector<float>("Weights", mesh.Weights, [&buf](size_t i, auto& v) {
    ImGui::SliderFloat(buf.Printf("##Weights_%zu", i), &v, 0, 1);
  });
}

// skin/node/scene/animation
void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Skin& skin)
{
  ShowGui("/skins", root.Skins.GetIndex(skin), skin);
  ImGui::BeginDisabled(true);
  SelectId("InverseBindMatrices", &skin.InverseBindMatrices, root.Accessors);
  SelectId("Skeleton", &skin.Skeleton, root.Nodes);
  ListId("Joints", skin.Joints, root.Nodes);
  ImGui::EndDisabled();
}

void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Node& node)
{
  ShowGui("/nodes", root.Nodes.GetIndex(node), node);
  // Id Camera;
  ListId("Children", node.Children, root.Nodes);
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

  PrintfBuffer buf;
  ShowGuiVector<float>("Weights", node.Weights, [&buf](size_t i, auto& value) {
    ImGui::SliderFloat(buf.Printf("##Weights_%zu", i), &value, 0, 1);
  });
}

void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Scene& scene)
{
  ShowGui("/scenes", root.Scenes.GetIndex(scene), scene);
  ListId("Nodes", scene.Nodes, root.Nodes);
}

void
ShowGui(const gltfjson::format::Root& root,
        const gltfjson::format::Bin& bin,
        gltfjson::format::Animation& animation)
{
  ShowGui("/animations", root.Animations.GetIndex(animation), animation);
}
