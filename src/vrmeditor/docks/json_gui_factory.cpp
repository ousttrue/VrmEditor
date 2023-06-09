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
      // { "/extensions/VRMC_springBone/springs/*/joints",
      // JsonGuiVrm1SpringJoints
      // },
      // { "/extensions/VRMC_springBone/colliders", JsonGuiVrm1SpringColliders
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
      { u8"/buffers/*", { u8"🔢", TypeFunc<gltfjson::Buffer>() } },
      { u8"/bufferViews/*", { u8"🔢", TypeFunc<gltfjson::BufferView>() } },
      { u8"/accessors", { u8"🔢", JsonGuiAccessorList } },
      { u8"/accessors/*", { u8"🔢", TypeFunc<gltfjson::Accessor>() } },
      // image/sampelr/texture/material
      // { u8"/images", { u8"🖼", JsonGuiImageList } },
      { u8"/images/*", { u8"🖼", TypeFunc<gltfjson::Image>() } },
      { u8"/samplers/*", { u8"🖼", TypeFunc<gltfjson::Sampler>() } },
      { u8"/textures/*", { u8"🖼", TypeFunc<gltfjson::Texture>() } },
      { u8"/textures/*/sampler", { u8"", SelectSampler } },
      { u8"/textures/*/source", { u8"", SelectTexture } },
      // { u8"/materials", { u8"🎨", JsonGuiMaterialList } },
      { u8"/materials/*", { u8"🎨", TypeFunc<gltfjson::Material>() } },
      {
        u8"/materials/*/alphaCutoff",
        { u8"", FloatSlider{ .Min = 0, .Max = 1, .Default = 0.5f } },
      },
      // mesh/skin
      // // { "/meshes", JsonGuiMeshList },
      { u8"/meshes/*", { u8"🔺", TypeFunc<gltfjson::Mesh>() } },
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
      // { u8"/skins", { u8"🔺", JsonGuiSkinList } },
      { u8"/skins/*", { u8"🔺", TypeFunc<gltfjson::Skin>() } },
      { u8"/skins/*/inverseBindMatrices", { u8"", JsonGuiAccessorReference } },
      // node/scene/animation/camera
      { u8"/nodes", { u8"✳ ", JsonGuiNodes } },
      { u8"/nodes/*", { u8"✳ ", TypeFunc<gltfjson::Node>() } },
      { u8"/scenes/*", { u8"✳ ", TypeFunc<gltfjson::Scene>() } },
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
  auto node = gltfjson::tree::FindJsonPath(root.m_json, m_selected);

  if (node) {
    std::visit(NodeTypeVisitor{}, node->Var);
    ImGui::SameLine();
  }
  // json path
  ImGui::TextUnformatted((const char*)m_selected.c_str());

  if (!m_cache) {
    if (auto match = m_guiFactories.Match(m_selected)) {
      m_cache = match->Editor(m_selected);
    } else {
      m_cache = [](auto& root, auto& bin, auto& node) {
        return std::visit(NodeEditorVisitor{}, node->Var);
      };
    }
  }
  if (node) {
    if (m_cache(root, bin, node)) {
      RaiseUpdated(m_selected);
    }
  }
}
