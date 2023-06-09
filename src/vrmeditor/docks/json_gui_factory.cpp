#include "json_gui_factory.h"
#include "json_gui_vrm0.h"
#include "jsonpath_gui.h"
#include "jsonpath_gui_accessor.h"
#include "jsonpath_gui_node.h"
#include "type_gui.h"
#include "type_gui_vrm0.h"
#include <gltfjson.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

template<typename T>
inline CreateGuiFunc
TypeFunc()
{
  return [](auto jsonapath) {
    return [](const gltfjson::Root& root,
              const gltfjson::Bin& bin,
              const gltfjson::tree::NodePtr& node) {
      ImGui::PushID(node.get());
      auto updated = ::ShowGui(root, bin, T{ node });
      ImGui::PopID();
      return updated;
    };
  };
}

JsonGuiFactoryManager::JsonGuiFactoryManager()
  : m_guiFactories({
      { u8"/extensions", { u8"⭐" } },
      { u8"/extras", { u8"⭐" } },
      { u8"/extensionsUsed", { u8"⭐" } },
      //
      { u8"/extensions/VRMC_vrm", { u8"🌟" } },
      { u8"/extensions/VRMC_vrm/humanoid", { u8"👤" } },
      // { "/extensions/VRMC_springBone/springs/*/joints",
      // JsonGuiVrm1SpringJoints
      // },
      // { "/extensions/VRMC_springBone/colliders", JsonGuiVrm1SpringColliders
      //
      { u8"/extensions/VRM", { u8"🌟" } },
      { u8"/extensions/VRM/meta", { u8"📄" } },
      { u8"/extensions/VRM/humanoid", { u8"👤" } },
      { u8"/extensions/VRM/humanoid/humanBones/*", { u8"🦴" } },
      { u8"/extensions/VRM/humanoid/humanBones/*/node", { u8"⤴ " } },
      { u8"/extensions/VRM/blendShapeMaster", { u8"😀" } },
      { u8"/extensions/VRM/firstPerson", { u8"👀" } },
      { u8"/extensions/VRM/secondaryAnimation", { u8"🔗" } },
      { u8"/extensions/VRM/materialProperties", { u8"💎" } },
      { u8"/extensions/VRM/materialProperties/*", { u8"💎" } },
      {
        u8"/extensions/VRM/materialProperties/*/vectorProperties/_Color",
        { u8"💎", RgbaPicker{} },
      },
      {
        u8"/extensions/VRM/materialProperties/*/vectorProperties/_ShadeColor",
        { u8"💎", RgbaPicker{} },
      },
      {
        u8"/extensions/VRM/materialProperties/*/vectorProperties/"
        u8"_EmissionColor",
        { u8"💎", RgbaPicker{} },
      },
      {
        u8"/extensions/VRM/materialProperties/*/vectorProperties/"
        u8"_OutlineColor",
        { u8"💎", RgbaPicker{} },
      },
      // // { "/extensions/VRM/secondaryAnimation/colliderGroups/*/colliders",
      // //   JsonGuiVrm0ColliderList },
      //
      // { u8"/extensions/VRM", TypeFunc<gltfjson::vrm0::VRM>() },
      // { u8"/extensions/VRM/meta", TypeFunc<gltfjson::vrm0::Meta>() },
      // { u8"/extensions/VRM/humanoid", TypeFunc<gltfjson::vrm0::Humanoid>() },
      // { u8"/extensions/VRM/firstPerson",
      //   TypeFunc<gltfjson::vrm0::FirstPerson>() },
      // { u8"/extensions/VRM/blendShapeMaster/blendShapeGroups/*",
      //   TypeFunc<gltfjson::vrm0::BlendShapeGroup>() },
      // { u8"/extensions/VRM/secondaryAnimation/boneGroups",
      //   JsonGuiVrm0SpringList },
      // { u8"/extensions/VRM/secondaryAnimation/boneGroups/*",
      //   TypeFunc<gltfjson::vrm0::Spring>() },
      // { u8"/extensions/VRM/secondaryAnimation/colliderGroups/*",
      //   TypeFunc<gltfjson::vrm0::ColliderGroup>() },
      // { u8"/extensions/VRM/materialProperties/*",
      //   TypeFunc<gltfjson::vrm0::Material>() },
      { u8"/asset", { u8"📄", TypeFunc<gltfjson::Asset>() } },
      // buffer/bufferView/accessor
      { u8"/buffers", { u8"📦" } },
      { u8"/buffers/*", { u8"📦", TypeFunc<gltfjson::Buffer>() } },
      { u8"/bufferViews", { u8"📦" } },
      { u8"/bufferViews/*", { u8"📦", TypeFunc<gltfjson::BufferView>() } },
      { u8"/accessors", { u8"📦", JsonGuiAccessorList } },
      { u8"/accessors/*", { u8"📦", TypeFunc<gltfjson::Accessor>() } },
      // image/sampelr/texture/material
      { u8"/images", { u8"🖼" } },
      { u8"/images/*", { u8"🖼", TypeFunc<gltfjson::Image>() } },
      { u8"/samplers", { u8"🖼" } },
      { u8"/samplers/*", { u8"🖼", TypeFunc<gltfjson::Sampler>() } },
      { u8"/textures", { u8"🖼" } },
      { u8"/textures/*", { u8"🖼", TypeFunc<gltfjson::Texture>() } },
      { u8"/textures/*/sampler", { u8"", SelectSampler } },
      { u8"/textures/*/source", { u8"", SelectTexture } },
      { u8"/materials", { u8"💎" } },
      { u8"/materials/*",
        { u8"💎",
          TypeFunc<gltfjson::Material>(),
          "https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/"
          "schema/material.schema.json" } },
      {
        u8"/materials/*/pbrMetallicRoughness/baseColorFactor",
        { u8"🎨",
          RgbaPicker{ .Default = { 1, 1, 1, 1 } },
          "The factors for the base color of the material. This value defines "
          "linear multipliers for the sampled texels of the base color "
          "texture." },
      },
      {
        u8"/materials/*/alphaMode",
        {
          u8"👻",
          StringEnum{ .Values = { "OPAQUE", "MASK", "BLEND" } },
        },
      },
      {
        u8"/materials/*/alphaCutoff",
        { u8"👻",
          FloatSlider{ .Min = 0, .Max = 1, .Default = 0.5f },
          "The alpha cutoff value of the material." },
      },
      // mesh/skin
      { u8"/meshes", { u8"📐" } },
      // // { "/meshes", JsonGuiMeshList },
      { u8"/meshes/*", { u8"📐", TypeFunc<gltfjson::Mesh>() } },
      {
        u8"/meshes/*/primitives/*",
        { u8"", TypeFunc<gltfjson::MeshPrimitive>() },
      },
      { u8"/meshes/*/primitives/*/indices",
        { u8"", JsonGuiAccessorReference } },
      { u8"/meshes/*/primitives/*/attributes/POSITION",
        { u8"", JsonGuiAccessorReference } },
      { u8"/meshes/*/primitives/*/attributes/NORMAL",
        { u8"", JsonGuiAccessorReference } },
      { u8"/meshes/*/primitives/*/attributes/TEXCOORD_0",
        { u8"", JsonGuiAccessorReference } },
      { u8"/meshes/*/primitives/*/attributes/JOINTS_0",
        { u8"", JsonGuiAccessorReference } },
      { u8"/meshes/*/primitives/*/attributes/WEIGHTS_0",
        { u8"", JsonGuiAccessorReference } },
      { u8"/skins", { u8"📐" } },
      { u8"/skins/*", { u8"📐", TypeFunc<gltfjson::Skin>() } },
      { u8"/skins/*/inverseBindMatrices", { u8"", JsonGuiAccessorReference } },
      // node/scene/animation/camera
      { u8"/nodes", { u8"✳ ", JsonGuiNodes } },
      { u8"/nodes/*", { u8"✳ ", TypeFunc<gltfjson::Node>() } },
      { u8"/scenes", { u8"✳ " } },
      { u8"/scenes/*", { u8"✳ ", TypeFunc<gltfjson::Scene>() } },
      { u8"/scene", { u8"✳ " } },
      { u8"/animations", { u8"▶ " } },
      {
        u8"/animations/*",
        { u8"▶ ", TypeFunc<gltfjson::Animation>() },
      },
      // { u8"/cameras/*", { u8"🎥", TypeFunc<gltfjson::Camera>() } },
    })
{
}

struct NodeTypeVisitor
{
  void operator()(std::monostate) { ImGui::TextUnformatted("[null]"); }
  void operator()(bool) { ImGui::TextUnformatted("[bool]"); }
  void operator()(float) { ImGui::TextUnformatted("[number]"); }
  void operator()(const std::u8string&) { ImGui::TextUnformatted("[string]"); }
  void operator()(const gltfjson::tree::ArrayValue&)
  {
    ImGui::TextUnformatted("[array]");
  }
  void operator()(const gltfjson::tree::ObjectValue&)
  {
    ImGui::TextUnformatted("[object]");
  }
};

struct NodeEditorVisitor
{
  gltfjson::tree::NodePtr node;
  bool operator()(std::monostate) { return false; }
  bool operator()(bool& value) { return ImGui::Checkbox("##bool", &value); }
  bool operator()(float& value) { return ImGui::InputFloat("##float", &value); }
  bool operator()(std::u8string& value)
  {
    std::string tmp((const char*)value.data(), value.size());
    if (ImGui::InputText("##text", &tmp)) {
      value.assign((const char8_t*)tmp.data(), tmp.size());
      return true;
    }
    return false;
  }
  bool operator()(const gltfjson::tree::ArrayValue&)
  {
    ImGui::BeginDisabled(true);
    if (ImGui::Button("Add")) {
      // TODO
    }
    ImGui::EndDisabled();
    return false;
  }
  bool operator()(const gltfjson::tree::ObjectValue&)
  {
    ImGui::BeginDisabled(true);
    if (ImGui::Button("Add")) {
      // TODO
    }
    ImGui::EndDisabled();
    return false;
  }
};

void
JsonGuiFactoryManager::ShowGui(const gltfjson::Root& root,
                               const gltfjson::Bin& bin)
{
  auto node = gltfjson::tree::FindJsonPath(root.m_json, m_jsonpath);

  if (node) {
    std::visit(NodeTypeVisitor{}, node->Var);
    ImGui::SameLine();
  }
  // json path
  ImGui::TextUnformatted((const char*)m_jsonpath.c_str());

  if (!m_editor) {
    auto found = m_cacheMap.find(m_jsonpath);
    if (found != m_cacheMap.end() && found->second.Factory) {
      m_current = found->second;
      m_editor = found->second.Factory(m_jsonpath);
    } else {
      // default
      m_editor = [](auto& root, auto& bin, auto& node) {
        return std::visit(NodeEditorVisitor{}, node->Var);
      };
    }
  }

  if (m_current.Description.size()) {
    ImGui::TextWrapped("%s", m_current.Description.c_str());
  }

  if (node) {
    if (m_editor(root, bin, node)) {
      RaiseUpdated(m_jsonpath);
    }
  }
}
