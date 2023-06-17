#include <GL/glew.h>

#include "gl3renderer.h"
#include "material.h"
#include "rendering_env.h"
#include "shader_source.h"
#include <DirectXMath.h>
// #include <Remotery.h>
#include <boneskin/deformed_mesh.h>
#include <boneskin/skin.h>
#include <boneskin/skinning_manager.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <cuber/mesh.h>
#include <gltfjson/gltf_typing_vrm0.h>
#include <grapho/gl3/glsl_type_name.h>
#include <grapho/gl3/pbr.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/texture.h>
#include <grapho/gl3/ubo.h>
#include <grapho/gl3/vao.h>
#include <iostream>
#include <plog/Log.h>
#include <unordered_map>
#include <variant>
#include <vrm/fileutil.h>
#include <vrm/gltfroot.h>
#include <vrm/image.h>

#include "material_error.h"
#include "material_pbr_khronos.h"
#include "material_pbr_learn_opengl.h"
#include "material_shadow.h"
#include "material_three_vrm.h"
#include "material_unlit.h"

namespace glr {

static std::expected<std::shared_ptr<libvrm::Image>, std::string>
ParseImage(const gltfjson::Root& root,
           const gltfjson::Bin& bin,
           const gltfjson::Image& image)
{
  std::span<const uint8_t> bytes;
  if (auto bufferView = image.BufferViewId()) {
    if (auto buffer_view = bin.GetBufferViewBytes(root, *bufferView)) {
      bytes = *buffer_view;
    } else {
      return std::unexpected{ buffer_view.error() };
    }
  } else if (image.UriString().size()) {
    if (auto buffer_view = bin.Dir->GetBuffer(image.UriString())) {
      bytes = *buffer_view;
    } else {
      return std::unexpected{ buffer_view.error() };
    }
  } else {
    return std::unexpected{ "not bufferView nor uri" };
  }
  auto name = image.NameString();
  auto ptr = std::make_shared<libvrm::Image>(name);
  if (!ptr->Load(bytes)) {
    return std::unexpected{ "Image: fail to load" };
  }
  return ptr;
}

class Gl3Renderer
{
  std::unordered_map<uint32_t, std::shared_ptr<libvrm::Image>> m_imageMap;

  std::unordered_map<uint32_t, std::shared_ptr<grapho::gl3::Texture>>
    m_srgbTextureMap;
  std::unordered_map<uint32_t, std::shared_ptr<grapho::gl3::Texture>>
    m_linearTextureMap;
  std::vector<std::shared_ptr<Material>> m_materialMap;
  std::unordered_map<uint32_t, std::shared_ptr<grapho::gl3::Vao>> m_drawableMap;

  Material m_shadow;
  Material m_error;

  std::shared_ptr<ShaderSourceManager> m_shaderSource;

  std::unordered_map<uint32_t, std::shared_ptr<boneskin::BaseMesh>>
    m_baseMeshMap;

  struct NodeMesh
  {
    uint32_t NodeIndex;
    uint32_t MeshIndex;
  };
  std::vector<NodeMesh> m_meshNodes;

  std::vector<MaterialFactory> m_pbrFactories{
    { "KHRONOS_GLTF_PBR", MaterialFactory_Pbr_Khronos_GLTF },
    { "LOGL_PBR", MaterialFactory_Pbr_LearnOpenGL },
  };
  uint32_t m_pbrFactoriesCurrent = 0;

  std::vector<MaterialFactory> m_unlitFactories{
    { "KHRONOS_GLTF_PBR", MaterialFactory_Pbr_Khronos_GLTF },
    { "experimental simple", MaterialFactory_Unlit },
  };
  uint32_t m_unlitFactoriesCurrent = 0;

  std::vector<MaterialFactory> m_mtoon0Factories{
    { "three-vrm", MaterialFactory_MToon },
  };
  uint32_t m_mtoon0FactoriesCurrent = 0;

  std::vector<MaterialFactory> m_mtoon1Factories{
    { "three-vrm", MaterialFactory_MToon },
  };
  uint32_t m_mtoon1FactoriesCurrent = 0;

  Gl3Renderer()
    : m_shaderSource(new ShaderSourceManager)
  {
  }

  ~Gl3Renderer() {}

public:
  static Gl3Renderer& Instance()
  {
    static Gl3Renderer s_instance;
    return s_instance;
  }

  void Release()
  {
    m_imageMap.clear();
    m_materialMap.clear();
    m_srgbTextureMap.clear();
    m_linearTextureMap.clear();
    m_drawableMap.clear();
  }

  std::vector<std::shared_ptr<Material>>& MaterialMap()
  {
    return m_materialMap;
  }

  void ReleaseMaterial(uint32_t i)
  {
    if (i < m_materialMap.size()) {
      m_materialMap[i] = {};
    }
  }

  // for local shader
  void SetShaderDir(const std::filesystem::path& path)
  {
    m_shaderSource->SetShaderDir(path);
    // clear cache
    m_materialMap.clear();
    m_shadow = {};
  }

  void SetShaderChunkDir(const std::filesystem::path& path)
  {
    m_shaderSource->SetShaderChunkDir(path);
  }

  // for hot reload
  // use relative path. pbr.{vs,fs}, unlit.{vs,fs}, mtoon.{vs,fs}
  void UpdateShader(const std::filesystem::path& path)
  {
    auto list = m_shaderSource->UpdateShader(path);
    for (auto type : list) {
    }
    // clear
    m_materialMap.clear();
    m_shadow = {};
    m_error = {};
  }

  std::shared_ptr<libvrm::Image> GetOrCreateImage(const gltfjson::Root& root,
                                                  const gltfjson::Bin& bin,
                                                  std::optional<uint32_t> id)
  {
    if (!id) {
      return {};
    }

    auto found = m_imageMap.find(*id);
    if (found != m_imageMap.end()) {
      return found->second;
    }

    if (auto image = ParseImage(root, bin, root.Images[*id])) {
      m_imageMap.insert({ *id, *image });
      return *image;
    } else {
      PLOG_ERROR << "image#" << *id << ": " << image.error();
      return {};
    }
  }

  std::shared_ptr<grapho::gl3::Texture> CreateTexture(
    const std::shared_ptr<libvrm::Image>& image)
  {
    auto texture = grapho::gl3::Texture::Create({
      image->Width(),
      image->Height(),
      grapho::PixelFormat::u8_RGBA,
      grapho::ColorSpace::Linear,
      image->Pixels(),
    });
    return texture;
  }

  std::shared_ptr<grapho::gl3::Texture> GetOrCreateTexture(
    const gltfjson::Root& root,
    const gltfjson::Bin& bin,
    std::optional<uint32_t> id,
    ColorSpace colorspace)
  {
    if (!id) {
      return {};
    }

    auto& map =
      colorspace == ColorSpace::sRGB ? m_srgbTextureMap : m_linearTextureMap;

    auto found = map.find(*id);
    if (found != map.end()) {
      return found->second;
    }

    auto src = root.Textures[*id];
    auto source = src.SourceId();
    if (!source) {
      return {};
    }

    auto image = GetOrCreateImage(root, bin, *source);
    if (!image) {
      return {};
    }

    auto texture = grapho::gl3::Texture::Create({
      image->Width(),
      image->Height(),
      grapho::PixelFormat::u8_RGBA,
      colorspace == ColorSpace::sRGB ? grapho::ColorSpace::sRGB
                                     : grapho::ColorSpace::Linear,
      image->Pixels(),
    });

    if (auto samplerIndex = src.SamplerId()) {
      auto sampler = root.Samplers[*samplerIndex];
      texture->Bind();
      glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        gltfjson::value_or<int>(sampler.MagFilter(),
                                (int)gltfjson::TextureMagFilter::LINEAR));
      glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        gltfjson::value_or<int>(sampler.MinFilter(),
                                (int)gltfjson::TextureMinFilter::LINEAR));
      glTexParameteri(GL_TEXTURE_2D,
                      GL_TEXTURE_WRAP_S,
                      gltfjson::value_or<int>(
                        sampler.WrapS(), (int)gltfjson::TextureWrap::REPEAT));
      glTexParameteri(GL_TEXTURE_2D,
                      GL_TEXTURE_WRAP_T,
                      gltfjson::value_or<int>(
                        sampler.WrapT(), (int)gltfjson::TextureWrap::REPEAT));
      texture->Unbind();
    } else {
      // TODO: default sampler
    }

    map.insert(std::make_pair(*id, texture));
    return texture;
  }

  std::shared_ptr<Material> GetOrCreateMaterial(const gltfjson::Root& root,
                                                const gltfjson::Bin& bin,
                                                std::optional<uint32_t> id)
  {
    if (!id) {
      return {};
    }

    if (*id < m_materialMap.size()) {
      if (auto found = m_materialMap[*id]) {
        return found;
      }
    }

    auto src = root.Materials[*id];

    auto extensions = src.Extensions();

    gltfjson::tree::NodePtr unlit;
    if (extensions) {
      unlit = extensions->Get(u8"KHR_materials_unlit");
    }

    gltfjson::tree::NodePtr mtoon1;
    if (extensions) {
      mtoon1 = extensions->Get(u8"VRMC_materials_mtoon");
    }

    gltfjson::tree::NodePtr mtoon0;
    if (auto root_extensins = root.Extensions()) {
      if (auto VRM = root_extensins->Get(u8"VRM")) {
        if (auto props = VRM->Get(u8"materialProperties")) {
          if (auto array = props->Array()) {
            if (*id < array->size()) {
              auto mtoonMaterial = (*array)[*id];
              if (auto shader = mtoonMaterial->Get(u8"shader")) {
                if (shader->U8String() == u8"VRM/MToon") {
                  mtoon0 = mtoonMaterial;
                }
              }
            }
          }
        }
      }
    }

    std::shared_ptr<Material> material;
    if (mtoon1) {
      material = m_mtoon1Factories[m_mtoon1FactoriesCurrent](root, bin, id);
    } else if (mtoon0) {
      material = m_mtoon0Factories[m_mtoon0FactoriesCurrent](root, bin, id);
    } else if (unlit) {
      material = m_unlitFactories[m_unlitFactoriesCurrent](root, bin, id);
    } else {
      material = m_pbrFactories[m_pbrFactoriesCurrent](root, bin, id);
    }

    while (*id >= m_materialMap.size()) {
      m_materialMap.push_back({});
    }
    m_materialMap[*id] = material;
    return material;
  }

  std::shared_ptr<grapho::gl3::Vao> GetOrCreateMesh(
    uint32_t id,
    const std::shared_ptr<boneskin::BaseMesh>& mesh)
  {
    assert(mesh);

    auto found = m_drawableMap.find(id);
    if (found != m_drawableMap.end()) {
      return found->second;
    }

    // load gpu resource
    auto vbo =
      grapho::gl3::Vbo::Create(mesh->verticesBytes(), mesh->m_vertices.data());
    auto ibo = grapho::gl3::Ibo::Create(
      mesh->indicesBytes(), mesh->m_indices.data(), GL_UNSIGNED_INT);

    std::shared_ptr<grapho::gl3::Vbo> slots[] = {
      vbo,
    };
    grapho::VertexLayout layouts[] = {
      {
        .Id = { 0, 0, "vPosition" },
        .Type = grapho::ValueType::Float,
        .Count = 3,
        .Offset = offsetof(boneskin::Vertex, Position),
        .Stride = sizeof(boneskin::Vertex),
      },
      {
        .Id = { 1, 0, "vNormal" },
        .Type = grapho::ValueType::Float,
        .Count = 3,
        .Offset = offsetof(boneskin::Vertex, Normal),
        .Stride = sizeof(boneskin::Vertex),
      },
      {
        .Id = { 2, 0, "vUv" },
        .Type = grapho::ValueType::Float,
        .Count = 2,
        .Offset = offsetof(boneskin::Vertex, Uv),
        .Stride = sizeof(boneskin::Vertex),
      },
    };
    auto vao = grapho::gl3::Vao::Create(layouts, slots, ibo);

    m_drawableMap.insert({ id, vao });

    return vao;
  }

  void RenderPasses(std::span<const RenderPass> passes,
                    const RenderingEnv& env,
                    const gltfjson::Root& root,
                    const gltfjson::Bin& bin,
                    std::span<const libvrm::DrawItem> drawables)
  {
    // rmt_ScopedCPUSample(RenderPasses, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    assert(root.Nodes.size() == drawables.size());
    m_meshNodes.clear();

    for (uint32_t i = 0; i < drawables.size(); ++i) {
      auto gltfNode = root.Nodes[i];
      if (auto meshId = gltfNode.MeshId()) {
        m_meshNodes.push_back({ i, *meshId });
        if (auto baseMesh =
              boneskin::SkinningManager::Instance().GetOrCreateBaseMesh(
                root, bin, meshId)) {

          std::span<const DirectX::XMFLOAT4X4> skinningMatrices;
          if (auto skin = boneskin::SkinningManager::Instance().GetOrCreaeSkin(
                root, bin, gltfNode.SkinId())) {
            // update skinnning
            // rmt_ScopedCPUSample(SkinningMatrices, 0);

            skin->CurrentMatrices.resize(skin->BindMatrices.size());

            auto rootInverse = DirectX::XMMatrixIdentity();
            if (auto root_index = skin->Root) {
              rootInverse = DirectX::XMMatrixInverse(
                nullptr, DirectX::XMLoadFloat4x4(&drawables[i].Matrix));
            }

            for (int i = 0; i < skin->Joints.size(); ++i) {
              auto m = skin->BindMatrices[i];
              DirectX::XMStoreFloat4x4(
                &skin->CurrentMatrices[i],
                DirectX::XMLoadFloat4x4(&m) *
                  DirectX::XMLoadFloat4x4(&drawables[skin->Joints[i]].Matrix) *
                  rootInverse);
            }

            skinningMatrices = skin->CurrentMatrices;
          }

          // upload vertices. CPU skinning and morpht target.
          auto deformed =
            boneskin::SkinningManager::Instance().GetOrCreateDeformedMesh(
              *meshId, baseMesh);
          if (deformed->Vertices.size()) {
            // apply morphtarget & skinning
            // rmt_ScopedCPUSample(SkinningApply, 0);
            deformed->ApplyMorphTargetAndSkinning(
              *baseMesh, drawables[i].MorphMap, skinningMatrices);
            auto vao = GetOrCreateMesh(*meshId, baseMesh);
            vao->slots_[0]->Upload(deformed->Vertices.size() *
                                     sizeof(boneskin::Vertex),
                                   deformed->Vertices.data());
          }
        }
      }
    }

    // render
    for (auto pass : passes) {
      for (auto& [nodeId, meshId] : m_meshNodes) {
        auto gltfNode = root.Nodes[nodeId];
        if (auto meshId = gltfNode.MeshId()) {
          if (auto baseMesh =
                boneskin::SkinningManager::Instance().GetOrCreateBaseMesh(
                  root, bin, meshId)) {
            auto vao = GetOrCreateMesh(*meshId, baseMesh);
            Render(
              pass, env, root, bin, baseMesh, vao, drawables[nodeId].Matrix);
          }
        }
      }
    }
  }

  void Render(RenderPass pass,
              const RenderingEnv& env,
              const gltfjson::Root& root,
              const gltfjson::Bin& bin,
              const std::shared_ptr<boneskin::BaseMesh>& baseMesh,
              const std::shared_ptr<grapho::gl3::Vao>& vao,
              const DirectX::XMFLOAT4X4& modelMatrix)
  {
    switch (pass) {
      case RenderPass::Opaque: {
        uint32_t drawOffset = 0;
        for (auto& primitive : baseMesh->m_primitives) {
          DrawPrimitive(false,
                        WorldInfo{ env },
                        LocalInfo{ modelMatrix },
                        root,
                        bin,
                        vao,
                        primitive,
                        drawOffset);
          drawOffset += primitive.DrawCount * 4;
        }
        break;
      }

      case RenderPass::Transparent: {
        // first: opaque
        uint32_t drawOffset = 0;
        for (auto& primitive : baseMesh->m_primitives) {
          DrawPrimitive(true,
                        WorldInfo{ env },
                        LocalInfo{ modelMatrix },
                        root,
                        bin,
                        vao,
                        primitive,
                        drawOffset);
          drawOffset += primitive.DrawCount * 4;
        }
        // second: transparent
        break;
      }

      case RenderPass::ShadowMatrix: {
        if (!m_shadow.Compiled) {
          if (auto shadow = MaterialFactory_Shadow(root, bin, {})) {
            m_shadow = *shadow;
          }
        }
        m_shadow.Activate(
          m_shaderSource, WorldInfo{ env }, LocalInfo{ modelMatrix }, {});
        uint32_t drawCount = 0;
        for (auto& primitive : baseMesh->m_primitives) {
          drawCount += primitive.DrawCount * 4;
        }
        vao->Draw(GL_TRIANGLES, drawCount, 0);
        break;
      }
    }
  }

  static bool MaterialIsTransparent(const gltfjson::tree::NodePtr& gltfMaterial,
                                    const gltfjson::tree::NodePtr& vrm0Material)
  {
    if (vrm0Material) {
      auto m = gltfjson::vrm0::Material(vrm0Material);
      if (auto p = m.BlendMode()) {
        if (*p == 2 || *p == 3) {
          return true;
        }
      }
    } else if (gltfMaterial) {
      auto m = gltfjson::Material(gltfMaterial);
      if (m.AlphaModeString() == u8"BLEND") {
        return true;
      }
    }

    return false;
  }

  static std::tuple<gltfjson::tree::NodePtr, gltfjson::tree::NodePtr>
  GetMaterial(const gltfjson::Root& root, std::optional<uint32_t> index)
  {
    if (!index) {
      return {};
    }

    if (*index >= root.Materials.size()) {
      return {};
    }

    return { root.Materials[*index].m_json,
             gltfjson::vrm0::GetVrmMaterial(root, *index) };
  }

  void DrawPrimitive(bool isTransparent,
                     const WorldInfo& world,
                     const LocalInfo& local,
                     const gltfjson::Root& root,
                     const gltfjson::Bin& bin,
                     const std::shared_ptr<grapho::gl3::Vao>& vao,
                     const boneskin::Primitive& primitive,
                     uint32_t drawOffset)
  {
    auto [gltfMaterial, vrm0Material] = GetMaterial(root, primitive.Material);

    if (MaterialIsTransparent(gltfMaterial, vrm0Material) != isTransparent) {
      return;
    }
    if (isTransparent) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      // glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_SRC_ALPHA,GL_ONE);
    }

    auto material_factory = GetOrCreateMaterial(root, bin, primitive.Material);
    if (material_factory) {

      material_factory->Activate(
        m_shaderSource, world, local, { root.m_json, *primitive.Material });

    } else {
      // error
      if (!m_error.Compiled) {
        if (auto error = MaterialFactory_Error(root, bin, primitive.Material)) {
          m_error = *error;
        }
      }
      m_error.Activate(m_shaderSource, world, local, {});
    }

    vao->Draw(GL_TRIANGLES, primitive.DrawCount, drawOffset);

    if (isTransparent) {
      glDisable(GL_BLEND);
    }
    assert(!grapho::gl3::TryGetError());
  }

  std::shared_ptr<grapho::gl3::PbrEnv> m_pbr;

  std::shared_ptr<grapho::gl3::Texture> LoadPbr_LOGL(
    const std::filesystem::path& path)
  {
    auto bytes = libvrm::ReadAllBytes(path);
    if (bytes.empty()) {
      PLOG_ERROR << "fail to read: " << path.string();
      return {};
    }

    auto hdr = std::make_shared<libvrm::Image>("pbr");
    if (!hdr->LoadHdr(bytes)) {
      PLOG_ERROR << "fail to load: " << path.string();
      return {};
    }

    auto texture = grapho::gl3::Texture::Create(
      {
        .Width = hdr->Width(),
        .Height = hdr->Height(),
        .Format = grapho::PixelFormat::f32_RGB,
        .ColorSpace = grapho::ColorSpace::Linear,
        .Pixels = hdr->Pixels(),
      },
      true);
    if (!texture) {
      return {};
    }

    m_pbr = std::make_shared<grapho::gl3::PbrEnv>(texture);
    return texture;
  }

  bool LoadPbr_Khronos(const std::filesystem::path& path) { return false; }

  bool LoadPbr_Threejs(const std::filesystem::path& path) { return false; }

  std::shared_ptr<grapho::gl3::Texture> GetEnvTexture(EnvTextureTypes type)
  {
    if (m_pbr) {
      switch (type) {
        case EnvTextureTypes::LOGL_BrdfLUT:
          return m_pbr->BrdfLUTTexture;
      }
    }
    return {};
  }

  std::shared_ptr<grapho::gl3::Cubemap> GetEnvCubemap(EnvCubemapTypes type)
  {
    if (m_pbr) {
      switch (type) {
        case EnvCubemapTypes::LOGL_IrradianceMap:
          return m_pbr->IrradianceMap;
        case EnvCubemapTypes::LOGL_PrefilterMap:
          return m_pbr->PrefilterMap;
      }
    }

    return {};
  }

  void RenderSkybox(const DirectX::XMFLOAT4X4& projection,
                    const DirectX::XMFLOAT4X4& view)
  {
    if (m_pbr) {
      m_pbr->DrawSkybox(projection, view);
    }
  }
};

void
Initialize()
{
  PLOG_INFO << "GL_VERSION: " << glGetString(GL_VERSION);
  PLOG_INFO << "GL_VENDOR: " << glGetString(GL_VENDOR);
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("glewInit");
  }
}

void
ClearBackBuffer(int width, int height)
{
  glViewport(0, 0, width, height);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
RenderPasses(std::span<const RenderPass> passes,
             const RenderingEnv& env,
             const gltfjson::Root& root,
             const gltfjson::Bin& bin,
             std::span<const libvrm::DrawItem> drawables)
{
  Gl3Renderer::Instance().RenderPasses(passes, env, root, bin, drawables);
}

void
ClearRendertarget(const RenderingEnv& env)
{
  grapho::gl3::ClearViewport({
    .Width = env.Width(),
    .Height = env.Height(),
    .Color = { env.PremulR(), env.PremulG(), env.PremulB(), env.Alpha() },
    .Depth = 1.0f,
  });
}

void
Release()
{
  Gl3Renderer::Instance().Release();
}

void
ReleaseMaterial(int i)
{
  Gl3Renderer::Instance().ReleaseMaterial(i);
}

std::shared_ptr<grapho::gl3::Texture>
GetOrCreateTexture(const gltfjson::Root& root,
                   const gltfjson::Bin& bin,
                   std::optional<uint32_t> texture,
                   ColorSpace colorspace)
{
  return Gl3Renderer::Instance().GetOrCreateTexture(
    root, bin, texture, colorspace);
}

std::optional<uint32_t>
GetOrCreateTextureHandle(const gltfjson::Root& root,
                         const gltfjson::Bin& bin,
                         std::optional<uint32_t> texture,
                         ColorSpace colorspace)
{
  if (auto p = GetOrCreateTexture(root, bin, texture, colorspace)) {
    return p->Handle();
  }
  return {};
}

void
RenderLine(const RenderingEnv& camera, std::span<const cuber::LineVertex> data)
{
  static cuber::gl3::GlLineRenderer s_liner;
  s_liner.Render(&camera.ProjectionMatrix._11, &camera.ViewMatrix._11, data);
}

// for local shader
void
SetShaderDir(const std::filesystem::path& path)
{
  Gl3Renderer::Instance().SetShaderDir(path);
}

// for threejs shaderchunk
void
SetShaderChunkDir(const std::filesystem::path& path)
{
  Gl3Renderer::Instance().SetShaderChunkDir(path);
}

// for hot reload
// use relative path. pbr.{vs,fs}, unlit.{vs,fs}, mtoon.{vs,fs}
void
UpdateShader(const std::filesystem::path& path)
{
  Gl3Renderer::Instance().UpdateShader(path);
}

std::shared_ptr<grapho::gl3::Texture>
LoadPbr_LOGL(const std::filesystem::path& path)
{
  return Gl3Renderer::Instance().LoadPbr_LOGL(path);
}

bool
LoadPbr_Khronos(const std::filesystem::path& path)
{
  return Gl3Renderer::Instance().LoadPbr_Khronos(path);
}

bool
LoadPbr_Threejs(const std::filesystem::path& path)
{
  return Gl3Renderer::Instance().LoadPbr_Threejs(path);
}

std::shared_ptr<grapho::gl3::Texture>
GetEnvTexture(EnvTextureTypes type)
{
  return Gl3Renderer::Instance().GetEnvTexture(type);
}

std::shared_ptr<grapho::gl3::Cubemap>
GetEnvCubemap(EnvCubemapTypes type)
{
  return Gl3Renderer::Instance().GetEnvCubemap(type);
}

void
RenderSkybox(const DirectX::XMFLOAT4X4& projection,
             const DirectX::XMFLOAT4X4& view)
{
  Gl3Renderer::Instance().RenderSkybox(projection, view);
}

std::vector<std::shared_ptr<Material>>&
MaterialMap()
{
  return Gl3Renderer::Instance().MaterialMap();
}

std::shared_ptr<grapho::gl3::Texture>
CreateTexture(const std::shared_ptr<libvrm::Image>& image)
{
  return Gl3Renderer::Instance().CreateTexture(image);
}

} // namespace
