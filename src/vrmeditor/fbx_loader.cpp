#include "fbx_loader.h"
#include <DirectXMath.h>
#include <boneskin/base_mesh.h>
#include <boneskin/meshdeformer.h>
#include <ufbx.h>
#include <vrm/node.h>

struct mesh_vertex
{
  DirectX::XMFLOAT3 position;
  DirectX::XMFLOAT3 normal;
  DirectX::XMFLOAT2 uv;
  float f_vertex_index;
};

struct skin_vertex
{
  uint8_t bone_index[4];
  uint8_t bone_weight[4];
};

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

  std::shared_ptr<libvrm::GltfRoot> Load(const std::filesystem::path& path)
  {
    ufbx_load_opts opts = { 0 }; // Optional, pass NULL for defaults
    m_scene = ufbx_load_file(path.string().c_str(), &opts, &m_error);
    if (!m_scene) {
      return {};
    }
    auto ptr = std::make_shared<libvrm::GltfRoot>();
    ptr->InitializeGltf();

    boneskin::SkinningManager::Instance().Release();
    for (size_t i = 0; i < m_scene->meshes.count; ++i) {
      auto mesh = m_scene->meshes.data[i];
      auto pMesh = std::make_shared<boneskin::BaseMesh>();
      boneskin::SkinningManager::Instance().PushBaseMesh(pMesh);

      // auto num_instances = mesh->instances.count;
      // auto instance_node_indices = alloc(int32_t, mesh->instances.count);
      for (size_t i = 0; i < mesh->instances.count; i++) {
        auto m = (int32_t)mesh->instances.data[i]->typed_id;
      }

      // Count the number of needed parts and temporary buffers
      size_t max_parts = 0;
      size_t max_triangles = 0;

      // We need to render each material of the mesh in a separate part, so
      // let's count the number of parts and maximum number of triangles needed.
      for (size_t pi = 0; pi < mesh->materials.count; pi++) {
        ufbx_mesh_material* mesh_mat = &mesh->materials.data[pi];
        if (mesh_mat->num_triangles == 0)
          continue;
        max_parts += 1;
        max_triangles =
          std::max<size_t>(max_triangles, mesh_mat->num_triangles);
      }

      size_t num_tri_indices = mesh->max_face_triangles * 3;
      std::vector<uint32_t> tri_indices(num_tri_indices);
      std::vector<mesh_vertex> vertices(max_triangles * 3);
      std::vector<skin_vertex> skin_vertices(max_triangles * 3);
      std::vector<skin_vertex> mesh_skin_vertices(mesh->num_vertices);
      std::vector<uint32_t> indices(max_triangles * 3);

      for (size_t pi = 0; pi < mesh->materials.count; pi++) {
        ufbx_mesh_material* mesh_mat = &mesh->materials.data[pi];
        size_t num_indices = 0;

        for (size_t fi = 0; fi < mesh_mat->num_faces; fi++) {
          ufbx_face face = mesh->faces.data[mesh_mat->face_indices.data[fi]];
          size_t num_tris = ufbx_triangulate_face(
            tri_indices.data(), tri_indices.size(), mesh, face);
          ufbx_vec2 default_uv = { 0 };

          for (size_t vi = 0; vi < num_tris * 3; vi++) {

            uint32_t ix = tri_indices[vi];
            mesh_vertex* vert = &vertices[num_indices];

            ufbx_vec3 pos = ufbx_get_vertex_vec3(&mesh->vertex_position, ix);
            ufbx_vec3 normal = ufbx_get_vertex_vec3(&mesh->vertex_normal, ix);
            ufbx_vec2 uv = mesh->vertex_uv.exists
                             ? ufbx_get_vertex_vec2(&mesh->vertex_uv, ix)
                             : default_uv;

            vert->position = { (float)pos.x, (float)pos.y, (float)pos.z };
            vert->normal = { (float)normal.x,
                             (float)normal.y,
                             (float)normal.z };
            vert->uv = { (float)uv.x, (float)uv.y };
            vert->f_vertex_index = (float)mesh->vertex_indices.data[ix];

            // The skinning vertex stream is pre-calculated above so we just
            // need to copy the right one by the vertex index.
            // if (skin) {
            //   skin_vertices[num_indices] =
            //     mesh_skin_vertices[mesh->vertex_indices.data[ix]];
            // }

            pMesh->m_vertices.push_back({
              { (float)pos.x, (float)pos.y, (float)pos.z },
              { (float)normal.x, (float)normal.y, (float)normal.z },
              { (float)uv.x, (float)uv.y },
            });

            num_indices++;
          }
        }
      }

      ptr->AddMesh(pMesh);
    }

    for (size_t i = 0; i < m_scene->nodes.count; i++) {
      auto node = m_scene->nodes.data[i];
      auto m = node->node_to_world;
      auto a = 0;

      std::string name{ (const char*)node->name.data, node->name.length };
      if (name.empty()) {
        std::stringstream ss;
        ss << "node#" << i;
        name = ss.str();
      }

      auto pNode = ptr->CreateNode(name);

      if (node->mesh) {
        for (size_t i = 0; i < m_scene->meshes.count; ++i) {
          auto mesh = m_scene->meshes.data[i];
          if (mesh == node->mesh) {
            auto a = 0;
          }
        }
      }
    }

    return ptr;
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

std::shared_ptr<libvrm::GltfRoot>
FbxLoader::Load(const std::filesystem::path& path)
{
  return m_impl->Load(path);
}

std::string
FbxLoader::Error() const
{
  return m_impl->m_error.description.data;
}
