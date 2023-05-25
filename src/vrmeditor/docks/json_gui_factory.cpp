#include "json_gui_factory.h"
#include <imgui.h>

JsonGuiFactoryManager::JsonGuiFactoryManager()
  : //
  m_guiFactories({
    //
    // { "/accessors", JsonGuiAccessorList },
    // { "/accessors/*", JsonGuiAccessor },
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
    // { "/extensions/VRM/secondaryAnimation/boneGroups", JsonGuiVrm0SpringList
    // },
    // { "/extensions/VRMC_springBone/springs/*/joints", JsonGuiVrm1SpringJoints
    // },
    // { "/extensions/VRMC_springBone/colliders", JsonGuiVrm1SpringColliders },
    //
  })
{
}

void
JsonGuiFactoryManager::ShowGui(const gltfjson::tree::NodePtr& gltf)
{
  ImGui::TextUnformatted((const char*)m_selected.c_str());
  if (!m_cache) {
    if (auto mached = MatchGui(m_selected)) {
      m_cache = (*mached)(gltf, m_selected);
    } else {
      m_cache = []() {};
    }
  }
  m_cache();
}
