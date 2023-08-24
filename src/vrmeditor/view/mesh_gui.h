#pragma once
#include <memory>

namespace libvrm {
struct GltfRoot;
}

class MeshGui
{
  struct MeshGuiImpl *m_impl; 
  
public:
  MeshGui();
  ~MeshGui();
  void ShowGui();
  void ShowView();
  void SetGltf(const std::shared_ptr<libvrm::GltfRoot>& root);
};
