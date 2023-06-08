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
      { u8"/asset", TypeFunc<gltfjson::Asset>() },
      // buffer/bufferView/accessor
      { u8"/buffers/*", TypeFunc<gltfjson::Buffer>() },
      { u8"/bufferViews/*", TypeFunc<gltfjson::BufferView>() },
      { u8"/accessors/*", TypeFunc<gltfjson::Accessor>() },
      // image/sampelr/texture/material
      { u8"/images/*", TypeFunc<gltfjson::Image>() },
      { u8"/samplers/*", TypeFunc<gltfjson::Sampler>() },
      { u8"/textures/*", TypeFunc<gltfjson::Texture>() },
      { u8"/textures/*/sampler", SelectSampler },
      { u8"/textures/*/source", SelectTexture },
      { u8"/materials/*", TypeFunc<gltfjson::Material>() },
      { u8"/materials/*/alphaCutoff",
        FloatSlider{ .Min = 0, .Max = 1, .Default = 0.5f } },
      // mesh/skin
      { u8"/meshes/*", TypeFunc<gltfjson::Mesh>() },
      { u8"/meshes/*/primitives/*", TypeFunc<gltfjson::MeshPrimitive>() },
      { u8"/skins/*", TypeFunc<gltfjson::Skin>() },
      // node/scene/animation/camera
      { u8"/nodes", JsonGuiNodes },
      { u8"/nodes/*", TypeFunc<gltfjson::Node>() },
      { u8"/scenes/*", TypeFunc<gltfjson::Scene>() },
      { u8"/animations/*", TypeFunc<gltfjson::Animation>() },
      // { u8"/cameras/*", TypeFunc<gltfjson::Camera>() },

      { u8"/accessors", JsonGuiAccessorList },
      { u8"/meshes/*/primitives/*/indices", JsonGuiAccessorReference },
      { u8"/meshes/*/primitives/*/attributes/POSITION",
        JsonGuiAccessorReference },
      { u8"/meshes/*/primitives/*/attributes/NORMAL",
        JsonGuiAccessorReference },
      { u8"/meshes/*/primitives/*/attributes/TEXCOORD_0",
        JsonGuiAccessorReference },
      { u8"/meshes/*/primitives/*/attributes/JOINTS_0",
        JsonGuiAccessorReference },
      { u8"/meshes/*/primitives/*/attributes/WEIGHTS_0",
        JsonGuiAccessorReference },
      { u8"/skins/*/inverseBindMatrices", JsonGuiAccessorReference },

      // { "/skins", JsonGuiSkinList },
      // { "/images", JsonGuiImageList },
      // { "/materials", JsonGuiMaterialList },
      // { "/meshes", JsonGuiMeshList },
      // { "/nodes", JsonGuiNodeList },
      // { "/extensions/VRM/secondaryAnimation/colliderGroups/*/colliders",
      //   JsonGuiVrm0ColliderList },

      { u8"/extensions/VRM", TypeFunc<gltfjson::vrm0::VRM>() },
      { u8"/extensions/VRM/meta", TypeFunc<gltfjson::vrm0::Meta>() },
      { u8"/extensions/VRM/humanoid", TypeFunc<gltfjson::vrm0::Humanoid>() },
      { u8"/extensions/VRM/firstPerson",
        TypeFunc<gltfjson::vrm0::FirstPerson>() },
      { u8"/extensions/VRM/blendShapeMaster/blendShapeGroups/*",
        TypeFunc<gltfjson::vrm0::BlendShapeGroup>() },
      { u8"/extensions/VRM/secondaryAnimation/boneGroups",
        JsonGuiVrm0SpringList },
      { u8"/extensions/VRM/secondaryAnimation/boneGroups/*",
        TypeFunc<gltfjson::vrm0::Spring>() },
      { u8"/extensions/VRM/secondaryAnimation/colliderGroups/*",
        TypeFunc<gltfjson::vrm0::ColliderGroup>() },
      { u8"/extensions/VRM/materialProperties/*",
        TypeFunc<gltfjson::vrm0::Material>() },

      // },
      // { "/extensions/VRMC_springBone/springs/*/joints",
      // JsonGuiVrm1SpringJoints
      // },
      // { "/extensions/VRMC_springBone/colliders", JsonGuiVrm1SpringColliders
      // },
      //
    })
{
}

struct NodeTypeVisitor
{
  void operator()(std::monostate) { ImGui::TextUnformatted("[null]"); }
  void operator()(bool) { ImGui::TextUnformatted("[bool]"); }
  void operator()(float) { ImGui::TextUnformatted("[float]"); }
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
      m_cache = (*match)(m_selected);
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
