#include <GL/glew.h>

#include "app.h"
#include "docks/printfbuffer.h"
#include "gl3renderer.h"
#include "gl3renderer_gui.h"
#include "material.h"
#include "rendering_env.h"
#include "shader_source.h"
#include <DirectXMath.h>
#include <TextEditor.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <cuber/mesh.h>
#include <gltfjson/gltf_typing_vrm0.h>
#include <grapho/gl3/glsl_type_name.h>
#include <grapho/gl3/pbr.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/texture.h>
#include <grapho/gl3/ubo.h>
#include <grapho/gl3/vao.h>
#include <grapho/imgui/csscolor.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <iostream>
#include <misc/cpp/imgui_stdlib.h>
#include <unordered_map>
#include <variant>
#include <vrm/deformed_mesh.h>
#include <vrm/fileutil.h>
#include <vrm/gltf.h>
#include <vrm/image.h>

#include "material_error.h"
#include "material_pbr_khronos.h"
#include "material_pbr_learn_opengl.h"
#include "material_shadow.h"
#include "material_three_vrm.h"
#include "material_unlit.h"

namespace glr {

static std::expected<std::shared_ptr<libvrm::gltf::Image>, std::string>
ParseImage(const gltfjson::Root& root,
           const gltfjson::Bin& bin,
           const gltfjson::Image& image)
{
  std::span<const uint8_t> bytes;
  if (auto bufferView = image.BufferView()) {
    if (auto buffer_view = bin.GetBufferViewBytes(root, *bufferView)) {
      bytes = *buffer_view;
    } else {
      return std::unexpected{ buffer_view.error() };
    }
  } else if (image.Uri().size()) {
    if (auto buffer_view = bin.Dir->GetBuffer(image.Uri())) {
      bytes = *buffer_view;
    } else {
      return std::unexpected{ buffer_view.error() };
    }
  } else {
    return std::unexpected{ "not bufferView nor uri" };
  }
  auto name = image.Name();
  auto ptr = std::make_shared<libvrm::gltf::Image>(name);
  if (!ptr->Load(bytes)) {
    return std::unexpected{ "Image: fail to load" };
  }
  return ptr;
}

class Gl3Renderer
{
  TextEditor m_vsEditor;
  TextEditor m_fsEditor;
  std::unordered_map<uint32_t, std::shared_ptr<libvrm::gltf::Image>> m_imageMap;
  std::unordered_map<uint32_t, std::shared_ptr<grapho::gl3::Texture>>
    m_srgbTextureMap;
  std::unordered_map<uint32_t, std::shared_ptr<grapho::gl3::Texture>>
    m_linearTextureMap;
  std::vector<std::shared_ptr<Material>> m_materialMap;
  std::unordered_map<uint32_t, std::shared_ptr<grapho::gl3::Vao>> m_drawableMap;

  std::shared_ptr<grapho::gl3::Texture> m_white;
  Material m_shadow;
  Material m_error;

  std::shared_ptr<ShaderSourceManager> m_shaderSource;

  struct MaterialFactory
  {
    std::string Name;
    MaterialFactoryFunc Factory;

    std::shared_ptr<Material> operator()(const gltfjson::Root& root,
                                         const gltfjson::Bin& bin,
                                         std::optional<uint32_t> materialId)
    {
      return Factory(root, bin, materialId);
    }
  };

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

  uint32_t m_selected = 0;

  Gl3Renderer()
    : m_shaderSource(new ShaderSourceManager)
  {
    static uint8_t white[] = { 255, 255, 255, 255 };
    m_white = grapho::gl3::Texture::Create({
      1,
      1,
      grapho::PixelFormat::u8_RGBA,
      grapho::ColorSpace::sRGB,
      white,
    });
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
    m_vsEditor.SetText("");
    m_fsEditor.SetText("");
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

  std::shared_ptr<libvrm::gltf::Image> GetOrCreateImage(
    const gltfjson::Root& root,
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
      App::Instance().Log(LogLevel::Error) << image.error();
      return {};
    }
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
    auto source = src.Source();
    if (!source) {
      return {};
    }

    auto image = GetOrCreateImage(root, bin, *source);

    auto texture = grapho::gl3::Texture::Create({
      image->Width(),
      image->Height(),
      grapho::PixelFormat::u8_RGBA,
      colorspace == ColorSpace::sRGB ? grapho::ColorSpace::sRGB
                                     : grapho::ColorSpace::Linear,
      image->Pixels(),
    });

    if (auto samplerIndex = src.Sampler()) {
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
    const std::shared_ptr<runtimescene::BaseMesh>& mesh)
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
        .Offset = offsetof(runtimescene::Vertex, Position),
        .Stride = sizeof(runtimescene::Vertex),
      },
      {
        .Id = { 1, 0, "vNormal" },
        .Type = grapho::ValueType::Float,
        .Count = 3,
        .Offset = offsetof(runtimescene::Vertex, Normal),
        .Stride = sizeof(runtimescene::Vertex),
      },
      {
        .Id = { 2, 0, "vUv" },
        .Type = grapho::ValueType::Float,
        .Count = 2,
        .Offset = offsetof(runtimescene::Vertex, Uv),
        .Stride = sizeof(runtimescene::Vertex),
      },
    };
    auto vao = grapho::gl3::Vao::Create(layouts, slots, ibo);

    m_drawableMap.insert({ id, vao });

    return vao;
  }

  void Render(RenderPass pass,
              const RenderingEnv& env,
              const gltfjson::Root& root,
              const gltfjson::Bin& bin,
              const gltfjson::tree::ArrayValue* vrm0Materials,
              uint32_t meshId,
              const std::shared_ptr<runtimescene::BaseMesh>& mesh,
              const runtimescene::DeformedMesh& deformed,
              const DirectX::XMFLOAT4X4& m)
  {
    glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LESS);

    auto vao = GetOrCreateMesh(meshId, mesh);
    // upload vertices. CPU skinning and morpht target.
    if (deformed.Vertices.size()) {
      vao->slots_[0]->Upload(deformed.Vertices.size() *
                               sizeof(runtimescene::Vertex),
                             deformed.Vertices.data());
    }

    switch (pass) {
      case RenderPass::Opaque: {
        uint32_t drawOffset = 0;
        for (auto& primitive : mesh->m_primitives) {
          DrawPrimitive(false,
                        WorldInfo{ env },
                        LocalInfo{ m },
                        root,
                        bin,
                        vrm0Materials,
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
        for (auto& primitive : mesh->m_primitives) {
          DrawPrimitive(true,
                        WorldInfo{ env },
                        LocalInfo{ m },
                        root,
                        bin,
                        vrm0Materials,
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
        m_shadow.Activate(m_shaderSource, WorldInfo{ env }, LocalInfo{ m }, {});
        uint32_t drawCount = 0;
        for (auto& primitive : mesh->m_primitives) {
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
      auto m = gltfjson::vrm0::Vrm0Material(vrm0Material);
      if (auto p = m.BlendMode()) {
        if (*p == 2 || *p == 3) {
          return true;
        }
      }
    } else if (gltfMaterial) {
      auto m = gltfjson::Material(gltfMaterial);
      if (m.AlphaMode() == u8"BLEND") {
        return true;
      }
    }

    return false;
  }

  void DrawPrimitive(bool isTransparent,
                     const WorldInfo& world,
                     const LocalInfo& local,
                     const gltfjson::Root& root,
                     const gltfjson::Bin& bin,
                     const gltfjson::tree::ArrayValue* vrm0Materials,
                     const std::shared_ptr<grapho::gl3::Vao>& vao,
                     const runtimescene::Primitive& primitive,
                     uint32_t drawOffset)
  {
    gltfjson::tree::NodePtr gltfMaterial;
    gltfjson::tree::NodePtr vrm0Material;
    if (primitive.Material) {
      if (vrm0Materials) {
        auto p = (*vrm0Materials)[*primitive.Material];
        if (auto name = p->Get(u8"shader")) {
          if (name->U8String() == u8"VRM/MToon") {
            vrm0Material = p;
          }
        }
      }
      gltfMaterial = root.Materials[*primitive.Material].m_json;
    }

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

      if (vrm0Material) {
        // VRM0+MToon
        material_factory->Activate(m_shaderSource, world, local, vrm0Material);
      } else {
        material_factory->Activate(m_shaderSource, world, local, gltfMaterial);
      }

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
    ERROR_CHECK;
  }

  void Select(uint32_t i)
  {
    if (i == m_selected) {
      return;
    }
    m_selected = i;
    if (auto material = m_materialMap[m_selected]) {
      m_vsEditor.SetText("");
      m_vsEditor.SetReadOnly(true);
      m_fsEditor.SetText("");
      m_fsEditor.SetReadOnly(true);
    }
  }

  static bool ShowSelectImpl(const std::vector<MaterialFactory>& list,
                             uint32_t* value)
  {
    bool updated = false;
    ImGui::Indent();
    for (uint32_t i = 0; i < list.size(); ++i) {
      auto& f = list[i];
      if (ImGui::Selectable(f.Name.c_str(), i == *value)) {
        *value = i;
        updated = true;
      }
    }
    ImGui::Unindent();
    return updated;
  }

  void ShowSelectImpl()
  {
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("PBR")) {
      if (ShowSelectImpl(m_pbrFactories, &m_pbrFactoriesCurrent)) {
        m_materialMap.clear();
      }
    }
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("UNLIT")) {
      if (ShowSelectImpl(m_unlitFactories, &m_unlitFactoriesCurrent)) {
        m_materialMap.clear();
      }
    }
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("MToon0")) {
      if (ShowSelectImpl(m_mtoon0Factories, &m_mtoon0FactoriesCurrent)) {
        m_materialMap.clear();
      }
    }
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("MToon1")) {
      if (ShowSelectImpl(m_mtoon1Factories, &m_mtoon1FactoriesCurrent)) {
        m_materialMap.clear();
      }
    }
  }

  void ShowSelector()
  {
    for (uint32_t i = 0; i < m_materialMap.size(); ++i) {
      PrintfBuffer buf;
      if (ImGui::Selectable(buf.Printf("%d", i), i == m_selected)) {
        Select(i);
      }
    }
  }

  void ShowSelectedShaderSource()
  {
    ImGui::Text("%d", m_selected);
    if (m_selected >= m_materialMap.size()) {
      return;
    }

    if (auto factory = m_materialMap[m_selected]) {
      ShowShaderSource(*factory, m_vsEditor, m_fsEditor);
    } else {
      ImGui::TextUnformatted("nullopt");
    }
  }

  void ShowSelectedShaderVariables()
  {
    ImGui::Text("%d", m_selected);
    if (m_selected >= m_materialMap.size()) {
      return;
    }

    if (auto factory = m_materialMap[m_selected]) {
      ShowShaderVariables(*factory);
    }
  }

  std::shared_ptr<grapho::gl3::PbrEnv> m_pbr;

  bool LoadPbr_LOGL(const std::filesystem::path& path)
  {
    auto bytes = libvrm::fileutil::ReadAllBytes(path);
    if (bytes.empty()) {
      App::Instance().Log(LogLevel::Error) << "fail to read: " << path;
      return false;
    }

    auto hdr = std::make_shared<libvrm::gltf::Image>("pbr");
    if (!hdr->LoadHdr(bytes)) {
      App::Instance().Log(LogLevel::Error) << "fail to load: " << path;
      return false;
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
      return false;
    }

    m_pbr = std::make_shared<grapho::gl3::PbrEnv>(texture);
    return true;
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
Render(RenderPass pass,
       const RenderingEnv& env,
       const gltfjson::Root& root,
       const gltfjson::Bin& bin,
       const gltfjson::tree::ArrayValue* vrm0Materials,
       uint32_t meshId,
       const std::shared_ptr<runtimescene::BaseMesh>& mesh,
       const runtimescene::DeformedMesh& deformed,
       const DirectX::XMFLOAT4X4& m)
{
  Gl3Renderer::Instance().Render(
    pass, env, root, bin, vrm0Materials, meshId, mesh, deformed, m);
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

void
CreateDock(const AddDockFunc& addDock)
{
  addDock(grapho::imgui::Dock(
    "GL impl", []() { Gl3Renderer::Instance().ShowSelectImpl(); }));

  addDock(grapho::imgui::Dock("GL selector", []() {
    //
    Gl3Renderer::Instance().ShowSelector();
  }));

  addDock(grapho::imgui::Dock("GL selected shader source", []() {
    //
    Gl3Renderer::Instance().ShowSelectedShaderSource();
  }));

  addDock(grapho::imgui::Dock("GL selected shader variables", []() {
    //
    Gl3Renderer::Instance().ShowSelectedShaderVariables();
  }));
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

bool
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
}
