#include "fbx_loader.h"
#include <ufbx.h>

struct FbxLoaderImpl
{
  ufbx_scene* m_scene = nullptr;
  ufbx_error m_error = {};

  FbxLoaderImpl() {}
  ~FbxLoaderImpl()
  {
    if (m_scene) {
      ufbx_free_scene(m_scene);
    }
  }

  bool Load(const std::filesystem::path& path)
  {
    ufbx_load_opts opts = { 0 }; // Optional, pass NULL for defaults
    m_scene = ufbx_load_file(path.string().c_str(), &opts, &m_error);
    return m_scene != nullptr;
  }
};

FbxLoader::FbxLoader()
  : m_impl(new FbxLoaderImpl)
{
}

FbxLoader::~FbxLoader()
{
  delete m_impl;
}

bool
FbxLoader::Load(const std::filesystem::path& path)
{
  return m_impl->Load(path);
}

std::string
FbxLoader::Error() const
{
  return m_impl->m_error.description.data;
}
