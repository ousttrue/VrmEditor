#include <GL/glew.h>

#include "app.h"
#include "glr/gl3renderer.h"
#include "im_widgets.h"
#include "type_gui.h"
#include "type_gui_accessor.h"
#include <array>
#include <misc/cpp/imgui_stdlib.h>
#include <sstream>
#include <unordered_map>

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Asset asset)
{
  ShowGuiString("copyright", asset.m_json, u8"copyright");
  ShowGuiString("generator", asset.m_json, u8"generator");
  ShowGuiString("version", asset.m_json, u8"version");
  ShowGuiString("minversion", asset.m_json, u8"minVersion");
  return false;
}

static bool
ShowGuiChildOfRoot(gltfjson::ChildOfRootProperty& prop)
{
  ShowGuiString("name", prop.m_json, u8"name");
  return false;
}

// buffer/bufferView/accessor
bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Buffer buffer)
{
  ShowGuiChildOfRoot(buffer);
  ImGui::BeginDisabled(true);
  ShowGuiString("Uri", buffer.m_json, u8"uri");
  ShowGuiUInt32("ByteLength", buffer.m_json, u8"byteLength");
  ImGui::EndDisabled();
  return false;
}

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::BufferView bufferView)
{
  ShowGuiChildOfRoot(bufferView);
  ImGui::BeginDisabled(true);

  SelectId(
    "Buffer", bufferView.m_json, u8"buffer", root.m_json->Get(u8"buffers"));
  ShowGuiUInt32("ByteOffset", bufferView.m_json, u8"byteOffset");
  ShowGuiUInt32("ByteLength", bufferView.m_json, u8"byteLength");
  ShowGuiUInt32("ByteStride", bufferView.m_json, u8"byteStride");
  ShowGuiEnum<gltfjson::Targets>(
    "Target", bufferView.m_json, u8"target", gltfjson::TargetsCombo);
  ImGui::EndDisabled();
  return false;
}

namespace std {
template<>
class hash<std::tuple<gltfjson::ComponentTypes, std::u8string>>
{
public:
  size_t operator()(
    const std::tuple<gltfjson::ComponentTypes, std::u8string>& x) const
  {
    return hash<int>()((int)std::get<0>(x)) ^
           hash<std::u8string>()(std::get<1>(x));
  }
};
}

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Accessor accessor)
{
  ShowGuiChildOfRoot(accessor);
  ImGui::BeginDisabled(true);
  SelectId(
    "BufferView", accessor.m_json, u8"bufferView", root.BufferViews.m_json);

  ShowGuiUInt32("ByteOffset", accessor.m_json, u8"byteOffset");

  ShowGuiEnum<gltfjson::ComponentTypes>("ComponentType",
                                        accessor.m_json,
                                        u8"componentType",
                                        gltfjson::ComponentTypesCombo);
  ShowGuiBool("Normalized", accessor.m_json, u8"normalized");
  ShowGuiUInt32("Count", accessor.m_json, u8"count");

  std::array<const char*, 7> TypesCombo = {
    "SCALAR", "VEC2", "VEC3", "VEC4", "MAT2", "MAT3", "MAT4",
  };
  if (ShowGuiStringEnum("Type", accessor.m_json, u8"type", TypesCombo)) {
  }

  ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
  ShowGuiVectorFloat("Max", accessor.m_json, u8"max", [](size_t i, float* p) {
    if (i) {
      ImGui::SameLine();
    }
    ImGui::Text("%f", p);
    return false;
  });

  ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
  ShowGuiVectorFloat("Min", accessor.m_json, u8"min", [](size_t i, float* p) {
    if (i) {
      ImGui::SameLine();
    }
    ImGui::Text("%f", p);
    return false;
  });

  ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
  ShowGuiOptional(accessor.m_json, u8"sparse", [](auto& node) {
    // TODO:
    // gltfjson::Sparse
    return false;
  });
  ImGui::EndDisabled();

  using ShowAccessorTable =
    std::function<void(const gltfjson::Root& root,
                       const gltfjson::Bin& bin,
                       const gltfjson::Accessor& accessor)>;

  using AccessorTypeKey = std::tuple<gltfjson::ComponentTypes, std::u8string>;
  static std::unordered_map<AccessorTypeKey, ShowAccessorTable> s_GuiMap = {
    { { gltfjson::ComponentTypes::UNSIGNED_BYTE, u8"SCALAR" },
      [](auto& root, auto& bin, auto& accessor) {
        if (auto values =
              bin.template GetAccessorBytes<uint8_t>(root, accessor)) {
          ShowGuiAccessorScalar<uint8_t>(*values);
        }
      } },
    { { gltfjson::ComponentTypes::UNSIGNED_SHORT, u8"SCALAR" },
      [](auto& root, auto& bin, auto& accessor) {
        if (auto values =
              bin.template GetAccessorBytes<uint16_t>(root, accessor)) {
          ShowGuiAccessorScalar<uint16_t>(*values);
        }
      } },
    { { gltfjson::ComponentTypes::UNSIGNED_SHORT, u8"VEC4" },
      [](auto& root, auto& bin, auto& accessor) {
        if (auto values = bin.template GetAccessorBytes<libvrm::ushort4>(
              root, accessor)) {
          ShowGuiAccessorInt4<libvrm::ushort4>(*values);
        }
      } },
    { { gltfjson::ComponentTypes::UNSIGNED_INT, u8"SCALAR" },
      [](auto& root, auto& bin, auto& accessor) {
        if (auto values =
              bin.template GetAccessorBytes<uint32_t>(root, accessor)) {
          ShowGuiAccessorScalar<uint32_t>(*values);
        }
      } },
    { { gltfjson::ComponentTypes::FLOAT, u8"VEC2" },
      [](auto& root, auto& bin, auto& accessor) {
        if (auto values = bin.template GetAccessorBytes<DirectX::XMFLOAT2>(
              root, accessor)) {
          ShowGuiAccessorVec2<DirectX::XMFLOAT2>(*values);
        }
      } },
    { { gltfjson::ComponentTypes::FLOAT, u8"VEC3" },
      [](auto& root, auto& bin, auto& accessor) {
        if (auto values = bin.template GetAccessorBytes<DirectX::XMFLOAT3>(
              root, accessor)) {
          ShowGuiAccessorVec3<DirectX::XMFLOAT3>(*values);
        }
      } },
    { { gltfjson::ComponentTypes::FLOAT, u8"VEC4" },
      [](auto& root, auto& bin, auto& accessor) {
        if (auto values = bin.template GetAccessorBytes<DirectX::XMFLOAT4>(
              root, accessor)) {
          ShowGuiAccessorVec4<DirectX::XMFLOAT4>(*values);
        }
      } },
    { { gltfjson::ComponentTypes::FLOAT, u8"MAT4" },
      [](auto& root, auto& bin, auto& accessor) {
        if (auto values = bin.template GetAccessorBytes<DirectX::XMFLOAT4X4>(
              root, accessor)) {
          ShowGuiAccessorMat4(*values);
        }
      } },
  };

  AccessorTypeKey key{ (gltfjson::ComponentTypes)*accessor.ComponentType(),
                       accessor.Type() };
  auto found = s_GuiMap.find(key);
  if (found != s_GuiMap.end()) {
    found->second(root, bin, accessor);
  }
  return false;
}

// image/sampler/texture/material/mesh
bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Image image)
{
  ShowGuiChildOfRoot(image);
  ImGui::BeginDisabled(true);
  ShowGuiString("Uri", image.m_json, u8"uri");
  ShowGuiString("MimeType", image.m_json, u8"mimeType");
  std::u8string MimeType;
  SelectId("BufferView", image.m_json, u8"bufferView", root.BufferViews.m_json);
  ImGui::EndDisabled();
  return false;
}

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Sampler sampler)
{
  ShowGuiChildOfRoot(sampler);
  ShowGuiEnum<gltfjson::TextureMagFilter>("magFilter",
                                          sampler.m_json,
                                          u8"magFilter",
                                          gltfjson::TextureMagFilterCombo);
  ShowGuiEnum<gltfjson::TextureMinFilter>("minFilter",
                                          sampler.m_json,
                                          u8"minFilter",
                                          gltfjson::TextureMinFilterCombo);
  ShowGuiEnum<gltfjson::TextureWrap>(
    "wrapS", sampler.m_json, u8"wrapS", gltfjson::TextureWrapCombo);
  ShowGuiEnum<gltfjson::TextureWrap>(
    "wrapT", sampler.m_json, u8"wrapT", gltfjson::TextureWrapCombo);
  return false;
}

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Texture texture)
{
  bool updated = false;
  if (ShowGuiChildOfRoot(texture)) {
    updated = true;
  }
  if (SelectId("Sampler", texture.m_json, u8"sampler", root.Samplers.m_json)) {
    updated = true;
  }
  if (SelectId("Source", texture.m_json, u8"source", root.Images.m_json)) {
    updated = true;
  }
  return updated;
}

static bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::TextureInfo info)
{
  bool updated = false;
  if (SelectId("Index", info.m_json, u8"index", root.Textures.m_json)) {
    updated = true;
  }

  ShowGuiTexturePreview(root, bin, info.m_json, u8"index");

  if (ShowGuiUInt32("TexCoord", info.m_json, u8"texCoord")) {
    updated = true;
  }
  return updated;
}

static bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::NormalTextureInfo info)
{
  return ShowGui(root, bin, *static_cast<gltfjson::TextureInfo*>(&info));
}

static bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::OcclusionTextureInfo info)
{
  return ShowGui(root, bin, *static_cast<gltfjson::TextureInfo*>(&info));
}

static bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::PbrMetallicRoughness pbr)
{
  bool updated = false;
  if (ShowGuiColor4(
        "BaseColorFactor", pbr.m_json, u8"baseColorFactor", { 1, 1, 1, 1 })) {
    updated = true;
  }
  if (ShowGuiOptional(
        pbr.m_json, u8"baseColorTexture", [&root, &bin](auto& node) {
          return ::ShowGui(root, bin, gltfjson::TextureInfo{ node });
        })) {
    updated = true;
  }
  if (ShowGuiSliderFloat(
        "MetallicFactor", pbr.m_json, u8"metallicFactor", 0, 1, 1.0f)) {
    updated = true;
  }
  if (ShowGuiSliderFloat(
        "RoughnessFactor", pbr.m_json, u8"roughnessFactor", 0, 1, 1.0f)) {
    updated = true;
  }

  HelpMarker(
    "The metallic-roughness texture. The metalness values are sampled from "
    "the B channel. The roughness values are sampled from the G channel. "
    "These values **MUST** be encoded with a linear transfer function. If "
    "other channels are present (R or A), they **MUST** be ignored for "
    "metallic-roughness calculations. When undefined, the texture **MUST** "
    "be sampled as having `1.0` in G and B components.");
  if (ShowGuiOptional(
        pbr.m_json, u8"metallicRoughnessTexture", [&root, &bin](auto node) {
          return ::ShowGui(root, bin, gltfjson::TextureInfo{ node });
        })) {
    updated = true;
  }
  return updated;
}

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Material material)
{
  bool updated = false;
  if (ShowGuiChildOfRoot(material)) {
    updated = true;
  }

  if (ShowGuiOptional(
        material.m_json, u8"pbrMetallicRoughness", [&root, &bin](auto& pbr) {
          return ::ShowGui(root, bin, gltfjson::PbrMetallicRoughness{ pbr });
        })) {
    updated = true;
  }
  if (ShowGuiOptional(
        material.m_json, u8"normalTexture", [&root, &bin](auto& info) {
          return ::ShowGui(root, bin, gltfjson::NormalTextureInfo{ info });
        })) {
    updated = true;
  }

  HelpMarker("The occlusion texture. The occlusion values are linearly sampled "
             "from the R channel. Higher values indicate areas that receive "
             "full indirect lighting and lower values indicate no indirect "
             "lighting. If other channels are present (GBA), they **MUST** be "
             "ignored for occlusion calculations. When undefined, the material "
             "does not have an occlusion texture.");
  if (ShowGuiOptional(
        material.m_json, u8"occlusionTexture", [&root, &bin](auto& info) {
          return ::ShowGui(root, bin, gltfjson::OcclusionTextureInfo{ info });
        })) {
    updated = true;
  }

  if (ShowGuiOptional(
        material.m_json, u8"emissiveTexture", [&root, &bin](auto& info) {
          return ::ShowGui(root, bin, gltfjson::TextureInfo{ info });
        })) {
    updated = true;
  }

  if (ShowGuiColor3(
        "EmissiveFactor", material.m_json, u8"emissiveFactor", { 0, 0, 0 })) {
    updated = true;
  }

  std::array<const char*, 3> AlphaModesCombo = {
    "OPAQUE",
    "MASK",
    "BLEND",
  };
  if (ShowGuiStringEnum(
        "AlphaMode", material.m_json, u8"alphaMode", AlphaModesCombo)) {
    updated = true;
  }

  if (ShowGuiSliderFloat(
        "AlphaCutoff", material.m_json, u8"alphaCutoff", 0, 1, 0.5f)) {
    updated = true;
  }
  if (ShowGuiBool("DoubleSided", material.m_json, u8"doubleSided")) {
    updated = true;
  }
  return updated;
}

static bool
ShowGui(const gltfjson::Root& root, gltfjson::MeshPrimitiveAttributes attribute)
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
  return false;
}

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::MeshPrimitiveMorphTarget target)
{
  SelectId("POSITION", target.m_json, u8"POSITION", root.Accessors.m_json);
  SelectId("NORMAL", target.m_json, u8"NORMAL", root.Accessors.m_json);
  return false;
}

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::MeshPrimitive prim)
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
    ShowGuiEnum<gltfjson::MeshPrimitiveTopology>(
      "Mode", prim.m_json, u8"mode", gltfjson::MeshPrimitiveTopologyCombo);
    ImGui::EndDisabled();
  }
  ImGui::PopID();
  return false;
}

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Mesh mesh)
{
  ShowGuiChildOfRoot(mesh);

  PrintfBuffer buf;

  ShowGuiVectorFloat(
    "Weights", mesh.m_json, u8"weights", [&buf](size_t i, float* p) {
      return ImGui::SliderFloat(buf.Printf("##Weights_%zu", i), p, 0, 1);
    });
  return false;
}

// skin/node/scene/animation
bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Skin skin)
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
  return false;
}

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Node node)
{
  ShowGuiChildOfRoot(node);
  // Id Camera;
  // ListId("Children", node.Children, root.Nodes);
  SelectId("Skin", node.m_json, u8"skin", root.Nodes.m_json);
  ShowGuiOptional(node.m_json, u8"matrix", [](auto& m) {
    auto values = FillArray<16>(m,
                                {
                                  //
                                  1,
                                  0,
                                  0,
                                  0,
                                  //
                                  0,
                                  1,
                                  0,
                                  0,
                                  //
                                  0,
                                  0,
                                  1,
                                  0,
                                  //
                                  0,
                                  0,
                                  0,
                                  1,
                                });
    bool updated = false;
    if (ImGui::InputFloat4("##m0", values.data())) {
      updated = true;
    }
    if (ImGui::InputFloat4("##m1", values.data() + 4)) {
      updated = true;
    }
    if (ImGui::InputFloat4("##m2", values.data() + 8)) {
      updated = true;
    }
    if (ImGui::InputFloat4("##m3", values.data() + 12)) {
      updated = true;
    }
    m->Set(values);
    return updated;
  });
  SelectId("Mesh", node.m_json, u8"mesh", root.Meshes.m_json);
  ShowGuiOptional(node.m_json, u8"rotation", [](auto& r) {
    return ShowGuiFloat4("##rotation", r, std::array<float, 4>{ 1, 0, 0, 0 });
  });
  ShowGuiOptional(node.m_json, u8"scale", [](auto& s) {
    return ShowGuiFloat3("##scale", s, std::array<float, 3>{ 1, 1, 1 });
  });
  ShowGuiOptional(node.m_json, u8"translation", [](auto& t) {
    return ShowGuiFloat3("##translation", t, std::array<float, 3>{ 0, 0, 0 });
  });

  PrintfBuffer buf;
  ShowGuiVectorFloat(
    "Weights", node.m_json, u8"weights", [&buf](size_t i, float* p) {
      return ImGui::SliderFloat(buf.Printf("##Weights_%zu", i), p, 0, 1);
    });
  return false;
}

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Scene scene)
{
  ShowGuiChildOfRoot(scene);
  // ListId("Nodes", scene.Nodes, root.Nodes);
  return false;
}

bool
ShowGui(const gltfjson::Root& root,
        const gltfjson::Bin& bin,
        gltfjson::Animation animation)
{
  ShowGuiChildOfRoot(animation);
  return false;
}
