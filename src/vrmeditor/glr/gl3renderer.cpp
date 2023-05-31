#include <GL/glew.h>

#include "app.h"
#include "docks/printfbuffer.h"
#include "gl3renderer.h"
#include "material_factory.h"
#include "rendering_env.h"
#include "shader_source.h"
#include <DirectXMath.h>
#include <TextEditor.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <cuber/mesh.h>
#include <grapho/gl3/pbr.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/texture.h>
#include <grapho/gl3/ubo.h>
#include <grapho/gl3/vao.h>
#include <grapho/imgui/widgets.h>
#include <imgui.h>
#include <iostream>
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
ParseImage(const gltfjson::typing::Root& root,
           const gltfjson::typing::Bin& bin,
           const gltfjson::typing::Image& image)
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
  std::vector<std::shared_ptr<MaterialFactory>> m_materialMap;
  std::unordered_map<uint32_t, std::shared_ptr<grapho::gl3::Vao>> m_drawableMap;

  std::shared_ptr<grapho::gl3::Texture> m_white;
  MaterialFactory m_shadow;
  MaterialFactory m_error;

  std::shared_ptr<ShaderSourceManager> m_shaderSource;
  std::unordered_map<ShaderTypes, MaterialFactoryFunc> m_materialFactoryMap{
    { ShaderTypes::Error, MaterialFactory_Error },
    { ShaderTypes::Shadow, MaterialFactory_Shadow },
    // { ShaderTypes::Pbr, MaterialFactory_Pbr_LearnOpenGL },
    { ShaderTypes::Pbr, MaterialFactory_Pbr_Khronos },
    { ShaderTypes::Unlit, MaterialFactory_Unlit },
    { ShaderTypes::MToon1, MaterialFactory_MToon },
    { ShaderTypes::MToon0, MaterialFactory_MToon },
  };

  grapho::gl3::Material::WorldVars m_env = {};
  std::shared_ptr<grapho::gl3::Ubo> m_envUbo;
  grapho::gl3::Material::LocalVars m_draw = {};
  std::shared_ptr<grapho::gl3::Ubo> m_drawUbo;

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

    m_envUbo = grapho::gl3::Ubo::Create<grapho::gl3::Material::WorldVars>();
    m_drawUbo = grapho::gl3::Ubo::Create<grapho::gl3::Material::LocalVars>();
  }

  std::shared_ptr<MaterialFactory> CreateMaterial(
    ShaderTypes type,
    const gltfjson::typing::Root& root,
    const gltfjson::typing::Bin& bin,
    std::optional<uint32_t> materialId)
  {
    auto found = m_materialFactoryMap.find(type);
    if (found == m_materialFactoryMap.end()) {
      return {};
    }
    return (found->second)(root, bin, materialId);
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
      switch (type) {
        case ShaderTypes::Pbr:
        case ShaderTypes::Unlit:
        case ShaderTypes::MToon0:
        case ShaderTypes::MToon1:
          // clear
          m_materialMap.clear();
          break;
        case ShaderTypes::Shadow:
          m_shadow = {};
          break;

        case ShaderTypes::Error:
          m_error = {};
          break;
      }
    }
  }

  std::shared_ptr<libvrm::gltf::Image> GetOrCreateImage(
    const gltfjson::typing::Root& root,
    const gltfjson::typing::Bin& bin,
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
    const gltfjson::typing::Root& root,
    const gltfjson::typing::Bin& bin,
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
      glTexParameteri(GL_TEXTURE_2D,
                      GL_TEXTURE_MAG_FILTER,
                      gltfjson::typing::value_or<int>(
                        sampler.MagFilter(),
                        (int)gltfjson::format::TextureMagFilter::LINEAR));
      glTexParameteri(GL_TEXTURE_2D,
                      GL_TEXTURE_MIN_FILTER,
                      gltfjson::typing::value_or<int>(
                        sampler.MinFilter(),
                        (int)gltfjson::format::TextureMinFilter::LINEAR));
      glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        gltfjson::typing::value_or<int>(
          sampler.WrapS(), (int)gltfjson::format::TextureWrap::REPEAT));
      glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        gltfjson::typing::value_or<int>(
          sampler.WrapT(), (int)gltfjson::format::TextureWrap::REPEAT));
      texture->Unbind();
    } else {
      // TODO: default sampler
    }

    map.insert(std::make_pair(*id, texture));
    return texture;
  }

  std::shared_ptr<MaterialFactory> GetOrCreateMaterial(
    const gltfjson::typing::Root& root,
    const gltfjson::typing::Bin& bin,
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

    std::shared_ptr<MaterialFactory> material;
    if (mtoon1) {
      material = CreateMaterial(ShaderTypes::MToon1, root, bin, id);
    } else if (mtoon0) {
      material = CreateMaterial(ShaderTypes::MToon0, root, bin, id);
    } else if (unlit) {
      material = CreateMaterial(ShaderTypes::Unlit, root, bin, id);
    } else {
      material = CreateMaterial(ShaderTypes::Pbr, root, bin, id);
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
              const gltfjson::typing::Root& root,
              const gltfjson::typing::Bin& bin,
              uint32_t meshId,
              const std::shared_ptr<runtimescene::BaseMesh>& mesh,
              const runtimescene::DeformedMesh& deformed,
              const DirectX::XMFLOAT4X4& m)
  {
    if (env.m_pbr) {
      env.m_pbr->Activate();
    }

    glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LESS);

    auto vao = GetOrCreateMesh(meshId, mesh);

    if (deformed.Vertices.size()) {
      vao->slots_[0]->Upload(deformed.Vertices.size() *
                               sizeof(runtimescene::Vertex),
                             deformed.Vertices.data());
    }

    m_env.projection = env.ProjectionMatrix;
    m_env.view = env.ViewMatrix;
    m_env.camPos = {
      env.CameraPosition.x,
      env.CameraPosition.y,
      env.CameraPosition.z,
      1,
    };
    m_envUbo->Upload(m_env);
    m_envUbo->SetBindingPoint(0);
    m_draw.model = m;
    m_draw.CalcNormalMatrix();
    WorldInfo world{ env };

    switch (pass) {
      case RenderPass::Color: {
        uint32_t drawOffset = 0;
        for (auto& primitive : mesh->m_primitives) {
          DrawPrimitive(world, root, bin, vao, primitive, drawOffset);
          drawOffset += primitive.DrawCount * 4;
        }
        break;
      }

      case RenderPass::ShadowMatrix: {
        if (!m_shadow.Material) {
          if (auto shadow =
                CreateMaterial(ShaderTypes::Shadow, root, bin, {})) {
            m_shadow = *shadow;
          }
        }
        m_shadow.Activate(m_shaderSource, world, LocalInfo{ m_draw });
        uint32_t drawCount = 0;
        for (auto& primitive : mesh->m_primitives) {
          drawCount += primitive.DrawCount * 4;
        }
        vao->Draw(GL_TRIANGLES, drawCount, 0);
        break;
      }
    }
  }

  void DrawPrimitive(const WorldInfo& world,
                     const gltfjson::typing::Root& root,
                     const gltfjson::typing::Bin& bin,
                     const std::shared_ptr<grapho::gl3::Vao>& vao,
                     const runtimescene::Primitive& primitive,
                     uint32_t drawOffset)
  {
    auto material_factory = GetOrCreateMaterial(root, bin, primitive.Material);
    if (material_factory) {
      // update ubo
      auto gltfMaterial = root.Materials[*primitive.Material];
      if (auto cutoff = gltfMaterial.AlphaCutoff()) {
        m_draw.cutoff.x = *cutoff;
      }
      m_draw.color = { 1, 1, 1, 1 };
      if (auto pbr = gltfMaterial.PbrMetallicRoughness()) {
        if (pbr->BaseColorFactor.size() == 4) {
          m_draw.color.x = pbr->BaseColorFactor[0];
          m_draw.color.y = pbr->BaseColorFactor[1];
          m_draw.color.z = pbr->BaseColorFactor[2];
          m_draw.color.w = pbr->BaseColorFactor[3];
        }
      }
      m_drawUbo->Upload(m_draw);
      m_drawUbo->SetBindingPoint(1);

      LocalInfo local{ m_draw };
      material_factory->Activate(m_shaderSource, world, local);

      // state
      glEnable(GL_CULL_FACE);
      glFrontFace(GL_CCW);
      // glCullFace(GL_BGR);
      glEnable(GL_DEPTH_TEST);

      switch (auto alphaMode = GetAlphaMode(root, primitive.Material)) {
        case gltfjson::format::AlphaModes::Opaque:
          glDisable(GL_BLEND);
          break;
        case gltfjson::format::AlphaModes::Mask:
          glDisable(GL_BLEND);
          break;
        case gltfjson::format::AlphaModes::Blend:
          glEnable(GL_BLEND);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          break;
      }
      // m_program->Uniform("cutoff")->SetFloat(primitive.Material->AlphaCutoff);
      // m_program->Uniform("color")->SetFloat4(
      //   primitive.Material->Pbr.BaseColorFactor);

      // texture->Activate(0);
    } else {
      // error
      if (!m_error.Material) {
        if (auto error = CreateMaterial(
              ShaderTypes::Error, root, bin, primitive.Material)) {
          m_error = *error;
        }
      }
      m_error.Activate(m_shaderSource, world, LocalInfo(m_draw));
    }

    vao->Draw(GL_TRIANGLES, primitive.DrawCount, drawOffset);
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

  void ShowSelector()
  {
    // ImGui::Text("srgb: %zd(textures)", m_srgbTextureMap.size());
    // ImGui::Text("linear: %zd(textures)", m_linearTextureMap.size());
    // ImGui::SameLine();
    // ImGui::Text("%zd(meshes)", m_drawableMap.size());
    for (uint32_t i = 0; i < m_materialMap.size(); ++i) {
      PrintfBuffer buf;
      if (ImGui::Selectable(buf.Printf("%d", i), i == m_selected)) {
        Select(i);
      }
    }
  }

  void ShowSelected()
  {
    ImGui::Text("%d", m_selected);
    if (m_selected >= m_materialMap.size()) {
      return;
    }

    if (auto factory = m_materialMap[m_selected]) {
      switch (factory->Type) {
        case ShaderTypes::Pbr:
          ImGui::TextUnformatted("pbr");
          break;
        case ShaderTypes::Unlit:
          ImGui::TextUnformatted("unlit");
          break;
        case ShaderTypes::MToon0:
          ImGui::TextUnformatted("mtoon0");
          break;
        case ShaderTypes::MToon1:
          ImGui::TextUnformatted("mtoon1");
          break;
      }

      ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
      if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
        if (ImGui::BeginTabItem("VS")) {
          if (ShowShader(factory->VS, m_vsEditor)) {
            // factory->Compiled = {};
            factory->Material = {};
            // factory->VS.Expand(factory->Type, m_shaderSource);
            // m_vsEditor.SetText((const char*)factory->VS.FullSource.c_str());
            m_vsEditor.SetText("");
          }
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("FS")) {
          if (ShowShader(factory->FS, m_fsEditor)) {
            // factory->Compiled = {};
            factory->Material = {};
            // factory->FS.Expand(factory->Type, m_shaderSource);
            // m_fsEditor.SetText((const char*)factory->FS.FullSource.c_str());
            m_fsEditor.SetText("");
          }
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Error")) {
          if (factory->Material) {

          } else {
            ImGui::TextWrapped("error: %s", factory->Compiled.error().c_str());
          }
          ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
      }

    } else {
      ImGui::TextUnformatted("nullopt");
    }
  }

  static bool ShowShader(ShaderFactory& s, TextEditor& editor)
  {
    bool updated = false;
    ImGui::TextUnformatted(s.SourceName.c_str());

    std::vector<std::tuple<int, std::string>> combo;
    for (auto& e : s.Enums) {
      combo.clear();
      for (auto& kv : e.Values) {
        combo.push_back(
          { kv.Value,
            std::string{ (const char*)kv.Name.data(), kv.Name.size() } });
      }
      int value = std::get<int>(e.Selected.Value);
      if (grapho::imgui::GenericCombo<int>(
            (const char*)e.Selected.Name.c_str(), &value, combo)) {
        e.Selected.Value = value;
        updated = true;
      }
    }

    if (editor.GetText() == "\n") {
      editor.SetText((const char*)s.FullSource.c_str());
      editor.SetReadOnly(true);
    }
    editor.Render(s.SourceName.c_str());

    return updated;
  }
};

void
Render(RenderPass pass,
       const RenderingEnv& env,
       const gltfjson::typing::Root& root,
       const gltfjson::typing::Bin& bin,
       uint32_t meshId,
       const std::shared_ptr<runtimescene::BaseMesh>& mesh,
       const runtimescene::DeformedMesh& deformed,
       const DirectX::XMFLOAT4X4& m)
{
  Gl3Renderer::Instance().Render(
    pass, env, root, bin, meshId, mesh, deformed, m);
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
  // glViewport(0, 0, env.Width(), env.Height());
  // glClearColor(env.PremulR(), env.PremulG(), env.PremulB(), env.Alpha());
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
  addDock(grapho::imgui::Dock("renderer selector", []() {
    //
    Gl3Renderer::Instance().ShowSelector();
  }));

  addDock(grapho::imgui::Dock("renderer selected", []() {
    //
    Gl3Renderer::Instance().ShowSelected();
  }));
}

std::shared_ptr<grapho::gl3::Texture>
GetOrCreateTexture(const gltfjson::typing::Root& root,
                   const gltfjson::typing::Bin& bin,
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
}
