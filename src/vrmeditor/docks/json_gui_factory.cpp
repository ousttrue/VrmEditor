#include "json_gui_factory.h"
#include "type_gui.h"
#include <imgui.h>

template<typename T>
inline CreateGuiFunc
GenGuiFunc()
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
    //
    // { "/accessors", JsonGuiAccessorList },
    {
      u8"/accessors/*",
      GenGuiFunc<gltfjson::typing::Accessor>(),
    },
    // { "/images", JsonGuiImageList },
    // { "/materials", JsonGuiMaterialList },
    // { "/meshes", JsonGuiMeshList },
    // { "/meshes/*", JsonGuiMesh },
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
