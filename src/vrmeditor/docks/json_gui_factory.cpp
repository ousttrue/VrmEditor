#include "json_gui_factory.h"
#include "json_gui_accessor.h"
#include "type_gui.h"
#include "type_gui_vrm0.h"
#include <imgui.h>
// #include "json_gui_images.h"
// #include "json_gui_material.h"
// #include "json_gui_mesh.h"
// #include "json_gui_node.h"
// #include "json_gui_skin.h"
#include "json_gui_vrm0.h"
// #include "json_gui_vrm1.h"
#include <glr/gl3renderer.h>

template<typename T>
inline CreateGuiFunc
TypeFunc()
{
  return [](auto jsonapath) {
    return [](const gltfjson::typing::Root& root,
              const gltfjson::typing::Bin& bin,
              const gltfjson::tree::NodePtr& node) {
      ImGui::PushID(node.get());
      auto updated = ::ShowGui(root, bin, T{ node });
      ImGui::PopID();
      return updated;
    };
  };
}

JsonGuiFactoryManager::JsonGuiFactoryManager()
  : //
  m_guiFactories({
    { u8"/asset", TypeFunc<gltfjson::typing::Asset>() },
    // buffer/bufferView/accessor
    { u8"/buffers/*", TypeFunc<gltfjson::typing::Buffer>() },
    { u8"/bufferViews/*", TypeFunc<gltfjson::typing::BufferView>() },
    { u8"/accessors/*", TypeFunc<gltfjson::typing::Accessor>() },
    // image/sampelr/texture/material
    { u8"/images/*", TypeFunc<gltfjson::typing::Image>() },
    { u8"/samplers/*", TypeFunc<gltfjson::typing::Sampler>() },
    { u8"/textures/*", TypeFunc<gltfjson::typing::Texture>() },
    { u8"/materials/*", TypeFunc<gltfjson::typing::Material>() },
    // mesh/skin
    { u8"/meshes/*", TypeFunc<gltfjson::typing::Mesh>() },
    { u8"/meshes/*/primitives/*", TypeFunc<gltfjson::typing::MeshPrimitive>() },
    { u8"/skins/*", TypeFunc<gltfjson::typing::Skin>() },
    // node/scene/animation/camera
    { u8"/nodes/*", TypeFunc<gltfjson::typing::Node>() },
    { u8"/scenes/*", TypeFunc<gltfjson::typing::Scene>() },
    { u8"/animations/*", TypeFunc<gltfjson::typing::Animation>() },
    // { u8"/cameras/*", TypeFunc<gltfjson::typing::Camera>() },

    { u8"/accessors", &JsonGuiAccessorList },
    { u8"/meshes/*/primitives/*/indices", JsonGuiAccessorReference },
    { u8"/meshes/*/primitives/*/attributes/POSITION",
      JsonGuiAccessorReference },
    { u8"/meshes/*/primitives/*/attributes/NORMAL", JsonGuiAccessorReference },
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

    { u8"/extensions/VRM", TypeFunc<gltfjson::typing::vrm0::VRM>() },
    { u8"/extensions/VRM/meta", TypeFunc<gltfjson::typing::vrm0::Meta>() },
    { u8"/extensions/VRM/humanoid",
      TypeFunc<gltfjson::typing::vrm0::Humanoid>() },
    { u8"/extensions/VRM/firstPerson",
      TypeFunc<gltfjson::typing::vrm0::FirstPerson>() },
    { u8"/extensions/VRM/blendShapeMaster/blendShapeGroups/*",
      TypeFunc<gltfjson::typing::vrm0::BlendShapeGroup>() },
    { u8"/extensions/VRM/secondaryAnimation/boneGroups",
      JsonGuiVrm0SpringList },
    { u8"/extensions/VRM/secondaryAnimation/boneGroups/*",
      TypeFunc<gltfjson::typing::vrm0::Spring>() },
    { u8"/extensions/VRM/secondaryAnimation/colliderGroups/*",
      TypeFunc<gltfjson::typing::vrm0::ColliderGroup>() },
    { u8"/extensions/VRM/materialProperties/*",
      TypeFunc<gltfjson::typing::vrm0::Material>() },

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

void
JsonGuiFactoryManager::ShowGui(const gltfjson::typing::Root& root,
                               const gltfjson::typing::Bin& bin)
{
  ImGui::TextUnformatted((const char*)m_selected.c_str());
  if (!m_cache) {
    if (auto mached = MatchGui(m_selected)) {
      m_cache = (*mached)(m_selected);
    } else {
      m_cache = [](auto& root, auto& bin, auto& node) { return false; };
    }
  }
  auto node = gltfjson::tree::FindJsonPath(root.m_json, m_selected);
  if (node) {
    if (m_cache(root, bin, node)) {
      OnUpdated(m_selected);
    }
  }
}

void
JsonGuiFactoryManager::OnUpdated(std::u8string_view jsonpath)
{
  gltfjson::JsonPath path(jsonpath);
  auto [childOfRoot, i] = path.GetChildOfRootIndex();
  if (childOfRoot==u8"materials") {
    glr::ReleaseMaterial(i);
  }
}
