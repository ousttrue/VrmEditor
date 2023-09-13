#include <GL/glew.h>
#include <ktx.h>

#include "gl3renderer.h"
#include "material.h"
#include "rendering_env.h"
#include "shader_source.h"
#include <DirectXMath.h>
// #include <Remotery.h>
#include <boneskin/deformed_mesh.h>
#include <boneskin/meshdeformer.h>
#include <boneskin/skin.h>
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

static std::shared_ptr<libvrm::Image>
ParseImage(const gltfjson::Root& root,
           const gltfjson::Bin& bin,
           const gltfjson::Image& image)
{
  std::span<const uint8_t> bytes;
  if (auto bufferView = image.BufferViewId()) {
    if (auto buffer_view = bin.GetBufferViewBytes(root, *bufferView)) {
      bytes = *buffer_view;
    } else {
      // return std::unexpected{ buffer_view.error() };
      return {};
    }
  } else if (image.UriString().size()) {
    if (auto buffer_view = bin.Dir->GetBuffer(image.UriString())) {
      bytes = *buffer_view;
    } else {
      // return std::unexpected{ buffer_view.error() };
      return {};
    }
  } else {
    // return std::unexpected{ "not bufferView nor uri" };
    return {};
  }
  auto name = image.NameString();
  auto ptr = std::make_shared<libvrm::Image>(name);
  if (!ptr->Load(bytes)) {
    // return std::unexpected{ "Image: fail to load" };
    return {};
  }
  return ptr;
}

static std::shared_ptr<grapho::gl3::Texture>
LoadBasisu(const gltfjson::Root& root,
           const gltfjson::Bin& bin,
           uint32_t source)
{
  auto span = bin.GetImageBytes(root, source);
  if (!span) {
    return {};
  }

  ktxTexture2* ktx;
  auto result = ktxTexture2_CreateFromMemory(
    span->data(), span->size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx);
  if (result != KTX_SUCCESS) {
    return {};
  }

  if (ktxTexture2_NeedsTranscoding(ktx)) {
    // ktx_texture_transcode_fmt_e tf;

    // Using VkGetPhysicalDeviceFeatures or
    // GL_COMPRESSED_TEXTURE_FORMATS or extension queries, determine
    // what compressed texture formats are supported and pick a format.
    // For example vk::PhysicalDeviceFeatures deviceFeatures;
    // vkctx.gpu.getFeatures(&deviceFeatures);
    // if (deviceFeatures.textureCompressionETC2)
    //   tf = KTX_TTF_ETC2_RGBA;
    // else if (deviceFeatures.textureCompressionBC)
    //   tf = KTX_TTF_BC3_RGBA;
    // else {
    //   message << "Vulkan implementation does not support any
    //   available
    //   "
    //              "transcode target.";
    //   throw std::runtime_error(message.str());
    // }
    auto tf = KTX_TTF_ASTC_4x4_RGBA;

    result = ktxTexture2_TranscodeBasis(ktx, tf, 0);
    if (result != KTX_SUCCESS) {
      return {};
    }
  }

  auto texture = std::make_shared<grapho::gl3::Texture>();
  GLenum target;
  GLenum error;
  result = ktxTexture_GLUpload(
    (ktxTexture*)ktx, (GLuint*)&texture->Handle(), &target, &error);
  if (result != KTX_SUCCESS) {
    return {};
  }

  return texture;
}

class Gl3Renderer
{
  std::unordered_map<uint32_t, std::shared_ptr<libvrm::Image>> m_imageMap;

  std::unordered_map<std::shared_ptr<libvrm::Image>,
                     std::shared_ptr<grapho::gl3::Texture>>
    m_srgbTextureMap;
  std::unordered_map<std::shared_ptr<libvrm::Image>,
                     std::shared_ptr<grapho::gl3::Texture>>
    m_linearTextureMap;
  std::vector<std::shared_ptr<Material>> m_materialMap;
  std::unordered_map<uint32_t, std::shared_ptr<grapho::gl3::Vao>> m_drawableMap;

  Material m_shadow;
  std::shared_ptr<Material> m_error;
  Material m_wireframe;

  std::shared_ptr<ShaderSourceManager> m_shaderSource;

  std::unordered_map<uint32_t, std::shared_ptr<boneskin::BaseMesh>>
    m_baseMeshMap;

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

  void ClearShaderCache()
  {
    m_materialMap.clear();
    m_shadow = {};
    m_wireframe = {};
    m_error = {};
  }

  // for local shader
  void SetShaderDir(const std::filesystem::path& path)
  {
    m_shaderSource->SetShaderDir(path);
    ClearShaderCache();
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
    ClearShaderCache();
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
      m_imageMap.insert({ *id, image });
      return image;
    } else {
      PLOG_ERROR << "image#" << *id << ": "
                 << "error"; // image.error();
      return {};
    }
  }

  std::shared_ptr<grapho::gl3::Texture> CreateTexture(
    const libvrm::Image& image)
  {
    auto texture = grapho::gl3::Texture::Create({
      image.Width(),
      image.Height(),
      grapho::PixelFormat::u8_RGBA,
      grapho::ColorSpace::Linear,
      image.Pixels(),
    });
    return texture;
  }

  std::shared_ptr<grapho::gl3::Texture> GetOrCreateTexture(
    const std::shared_ptr<libvrm::Image>& image,
    ColorSpace colorspace)
  {
    if (!image) {
      return nullptr;
    }

    auto& map =
      colorspace == ColorSpace::sRGB ? m_srgbTextureMap : m_linearTextureMap;

    auto found = map.find(image);
    if (found != map.end()) {
      return found->second;
    }

    auto texture = grapho::gl3::Texture::Create({
      image->Width(),
      image->Height(),
      grapho::PixelFormat::u8_RGBA,
      colorspace == ColorSpace::sRGB ? grapho::ColorSpace::sRGB
                                     : grapho::ColorSpace::Linear,
      image->Pixels(),
    });

    map.insert(std::make_pair(image, texture));
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

    auto src = root.Textures[*id];
    std::shared_ptr<grapho::gl3::Texture> texture;
    if (auto KHR_texture_basisu =
          src.GetExtension<gltfjson::KHR_texture_basisu>()) {
      if (auto basisu = KHR_texture_basisu->SourceId()) {
        texture = LoadBasisu(root, bin, *basisu);
      }
    } else if (auto source = src.SourceId()) {
      if (auto image = GetOrCreateImage(root, bin, *source)) {
        texture = GetOrCreateTexture(image, colorspace);
      }
    }

    if (texture) {
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
      return texture;
    } else {
      return nullptr;
    }
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

    if (*id >= root.Materials.size()) {
      return m_error;
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
          if (auto array =
                std::dynamic_pointer_cast<gltfjson::tree::ArrayNode>(props)) {
            if (*id < array->Value.size()) {
              auto mtoonMaterial = array->Value[*id];
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
                    const grapho::camera::Camera& camera,
                    const RenderingEnv& env,
                    const gltfjson::Root& root,
                    const gltfjson::Bin& bin,
                    const MeshDrawInfo& draw)
  {
    // rmt_ScopedCPUSample(RenderPasses, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // render
    for (auto pass : passes) {
      auto vao = GetOrCreateMesh(draw.MeshId, draw.BaseMesh);

      vao->slots_[0]->Upload(draw.Vertices.size() * sizeof(boneskin::Vertex),
                             draw.Vertices.data());

      Render(pass, camera, env, root, bin, draw.BaseMesh, vao, draw.Matrix);
    }
  }

  void Render(RenderPass pass,
              const grapho::camera::Camera& camera,
              const RenderingEnv& env,
              const gltfjson::Root& root,
              const gltfjson::Bin& bin,
              const std::shared_ptr<boneskin::BaseMesh>& baseMesh,
              const std::shared_ptr<grapho::gl3::Vao>& vao,
              const DirectX::XMFLOAT4X4& modelMatrix)
  {
    if (!m_error) {
      if (auto error = MaterialFactory_Error(root, bin, {})) {
        m_error = error;
      }
    }

    switch (pass) {
      case RenderPass::Opaque: {
        uint32_t drawOffset = 0;
        for (auto& primitive : baseMesh->m_primitives) {
          DrawPrimitive(false,
                        WorldInfo{ camera, env },
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
                        WorldInfo{ camera, env },
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
        m_shadow.Activate(m_shaderSource,
                          WorldInfo{ camera, env },
                          LocalInfo{ modelMatrix },
                          {});
        uint32_t drawCount = 0;
        for (auto& primitive : baseMesh->m_primitives) {
          drawCount += primitive.DrawCount * 4;
        }
        vao->Draw(GL_TRIANGLES, drawCount, 0);
        break;
      }

      case RenderPass::Wireframe: {
        if (!m_wireframe.Compiled) {
          if (auto material = MaterialFactory_Wireframe(root, bin, {})) {
            m_wireframe = *material;
          }
        }
        m_wireframe.Activate(m_shaderSource,
                             WorldInfo{ camera, env },
                             LocalInfo{ modelMatrix },
                             {});
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
      m_error->Activate(m_shaderSource, world, local, {});
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
             const grapho::camera::Camera& camera,
             const RenderingEnv& env,
             const gltfjson::Root& root,
             const gltfjson::Bin& bin,
             const MeshDrawInfo& draw)
{
  Gl3Renderer::Instance().RenderPasses(passes, camera, env, root, bin, draw);
}

void
ClearRendertarget(const grapho::camera::Camera& camera, const RenderingEnv& env)
{
  grapho::gl3::ClearViewport({
    .Width = camera.Projection.Viewport.Width,
    .Height = camera.Projection.Viewport.Height,
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

std::optional<uint32_t>
GetOrCreateTextureHandle(const std::shared_ptr<libvrm::Image>& image,
                         ColorSpace colorspace)
{
  if (auto p = Gl3Renderer::Instance().GetOrCreateTexture(image, colorspace)) {
    return p->Handle();
  }
  return {};
}

std::shared_ptr<libvrm::Image>
GetOrCreateImage(const gltfjson::Root& root,
                 const gltfjson::Bin& bin,
                 std::optional<uint32_t> image)
{
  return Gl3Renderer::Instance().GetOrCreateImage(root, bin, image);
}

void
RenderLine(const grapho::camera::Camera& camera,
           std::span<const cuber::LineVertex> data)
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
CreateTexture(const libvrm::Image& image)
{
  return Gl3Renderer::Instance().CreateTexture(image);
}

} // namespace
