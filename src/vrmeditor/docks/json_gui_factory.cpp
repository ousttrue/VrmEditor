#include "json_gui_factory.h"
#include "type_gui.h"
#include <imgui.h>

template<typename T>
inline CreateGuiFunc
TypeFunc()
{
  return [](auto jsonapath) {
    return [](const gltfjson::typing::Root& root,
              const gltfjson::typing::Bin& bin,
              const gltfjson::tree::NodePtr& node) {
      ::ShowGui(root, bin, T{ node });
    };
  };
}

JsonGuiFactoryManager::JsonGuiFactoryManager()
  : //
  m_guiFactories({
    // buffer/bufferView/accessor
    { u8"/buffers/*", TypeFunc<gltfjson::typing::Buffer>() },
    { u8"/bufferViews/*", TypeFunc<gltfjson::typing::BufferView>() },
    { u8"/accessors/*", TypeFunc<gltfjson::typing::Accessor>() },
    // image/sampelr/texture/material
    { u8"/images/*", TypeFunc<gltfjson::typing::Image>() },
    { u8"/sampelrs/*", TypeFunc<gltfjson::typing::Sampler>() },
    { u8"/textures/*", TypeFunc<gltfjson::typing::Texture>() },
    { u8"/materials/*", TypeFunc<gltfjson::typing::Material>() },
    // mesh/skin
    { u8"/meshes/*", TypeFunc<gltfjson::typing::Mesh>() },
    { u8"/skins/*", TypeFunc<gltfjson::typing::Skin>() },
    // node/scene/animation/camera
    { u8"/nodes/*", TypeFunc<gltfjson::typing::Node>() },
    { u8"/scenes/*", TypeFunc<gltfjson::typing::Scene>() },
    { u8"/animations/*", TypeFunc<gltfjson::typing::Animation>() },
    // { u8"/cameras/*", TypeFunc<gltfjson::typing::Camera>() },

    // { "/accessors", JsonGuiAccessorList },
    // { "/images", JsonGuiImageList },
    // { "/materials", JsonGuiMaterialList },
    // { "/meshes", JsonGuiMeshList },
    // { "/meshes/*/primitives/*/attributes/POSITION", JsonGuiAccessor },
    // { "/meshes/*/primitives/*/attributes/NORMAL", JsonGuiAccessor },
    // { "/meshes/*/primitives/*/attributes/TEXCOORD_0", JsonGuiAccessor },
    // { "/meshes/*/primitives/*/attributes/JOINTS_0", JsonGuiAccessor },
    // { "/meshes/*/primitives/*/attributes/WEIGHTS_0", JsonGuiAccessor },
    // { "/skins", JsonGuiSkinList },
    // { "/skins/*/inverseBindMatrices", JsonGuiAccessor },
    // { "/nodes", JsonGuiNodeList },
    // { "/extensions/VRM/secondaryAnimation/colliderGroups/*/colliders",
    //   JsonGuiVrm0ColliderList },
    // { "/extensions/VRM/secondaryAnimation/boneGroups",
    // JsonGuiVrm0SpringList
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
      m_cache = [](auto& root, auto& bin, auto& node) {};
    }
  }
  auto node = gltfjson::tree::FindJsonPath(root.m_json, m_selected);
  if (node) {
    m_cache(root, bin, node);
  }
}
