#include "gl3renderer.h"
#include "app.h"
#include "rendering_env.h"
#include "shader_source.h"
#include <DirectXMath.h>
#include <GL/glew.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <cuber/mesh.h>
#include <grapho/gl3/pbr.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/texture.h>
#include <grapho/gl3/ubo.h>
#include <grapho/gl3/vao.h>
#include <imgui.h>
#include <iostream>
#include <unordered_map>
#include <variant>
#include <vrm/deformed_mesh.h>
#include <vrm/fileutil.h>
#include <vrm/gltf.h>
#include <vrm/image.h>

namespace glr {

static gltfjson::format::AlphaModes
GetAlphaMode(const gltfjson::typing::Root& root,
             std::optional<uint32_t> material)
{
  if (material) {
    if (auto alphaMode = root.Materials[*material].AlphaMode()) {
      return (gltfjson::format::AlphaModes)*alphaMode;
    }
  }
  return gltfjson::format::AlphaModes::Opaque;
}

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
  std::unordered_map<uint32_t, std::shared_ptr<libvrm::gltf::Image>> m_imageMap;
  std::unordered_map<uint32_t, std::shared_ptr<grapho::gl3::Texture>>
    m_srgbTextureMap;
  std::unordered_map<uint32_t, std::shared_ptr<grapho::gl3::Texture>>
    m_linearTextureMap;
  std::unordered_map<uint32_t, std::shared_ptr<grapho::gl3::Material>>
    m_materialMap;
  std::unordered_map<uint32_t, std::shared_ptr<grapho::gl3::Vao>> m_drawableMap;

  std::shared_ptr<grapho::gl3::Texture> m_white;
  std::shared_ptr<grapho::gl3::ShaderProgram> m_shadow;
  std::shared_ptr<grapho::gl3::ShaderProgram> m_error;

  ShaderSourceManager m_shaderSource;

  grapho::gl3::Material::EnvVars m_env = {};
  std::shared_ptr<grapho::gl3::Ubo> m_envUbo;
  grapho::gl3::Material::ModelVars m_model = {};
  std::shared_ptr<grapho::gl3::Ubo> m_modelUbo;

  Gl3Renderer()
  {
    static uint8_t white[] = { 255, 255, 255, 255 };
    m_white = grapho::gl3::Texture::Create({
      1,
      1,
      grapho::PixelFormat::u8_RGBA,
      grapho::ColorSpace::sRGB,
      white,
    });

    m_envUbo = grapho::gl3::Ubo::Create<grapho::gl3::Material::EnvVars>();
    m_modelUbo = grapho::gl3::Ubo::Create<grapho::gl3::Material::ModelVars>();

    m_shaderSource.Register(ShaderTypes::Error, { "error.vert", "error.frag" });
    m_shaderSource.Register(ShaderTypes::Pbr,
                            { "khronos/primitive.vert", "khronos/pbr.frag" });
    m_shaderSource.Register(ShaderTypes::Unlit,
                            { "khronos/primitive.vert", "khronos/pbr.frag" });
    m_shaderSource.Register(ShaderTypes::MToon1,
                            { "mtoon.vert", "mtoon.frag" });
    m_shaderSource.Register(ShaderTypes::MToon0,
                            { "mtoon.vert", "mtoon.frag" });
    m_shaderSource.Register(ShaderTypes::Shadow,
                            { "shadow.vert", "shadow.frag" });
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
    m_materialMap.clear();
    m_drawableMap.clear();
  }

  void ReleaseMaterial(int i) { m_materialMap.erase(i); }

  // for local shader
  void SetShaderDir(const std::filesystem::path& path)
  {
    m_shaderSource.SetShaderDir(path);
    // clear cache
    m_materialMap.clear();
    m_shadow = {};
  }

  void SetShaderChunkDir(const std::filesystem::path& path)
  {
    m_shaderSource.SetShaderChunkDir(path);
  }

  // for hot reload
  // use relative path. pbr.{vs,fs}, unlit.{vs,fs}, mtoon.{vs,fs}
  void UpdateShader(const std::filesystem::path& path)
  {
    auto list = m_shaderSource.UpdateShader(path);
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
    libvrm::gltf::ColorSpace colorspace)
  {
    if (!id) {
      return {};
    }

    auto& map = colorspace == libvrm::gltf::ColorSpace::sRGB
                  ? m_srgbTextureMap
                  : m_linearTextureMap;

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
      colorspace == libvrm::gltf::ColorSpace::sRGB ? grapho::ColorSpace::sRGB
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

  std::shared_ptr<grapho::gl3::Material> CreateMaterialMToon0(
    const gltfjson::typing::Root& root,
    const gltfjson::typing::Bin& bin,
    const gltfjson::tree::NodePtr& mtoon)
  {
    std::vector<std::u8string_view> vs;
    std::vector<std::u8string_view> fs;
    vs.push_back(u8"#version 300 es\n");
    vs.push_back(u8"#define THREE_VRM_THREE_REVISION 150\n");
    vs.push_back(u8"#define NUM_SPOT_LIGHT_COORDS 4\n");
    vs.push_back(u8"#define NUM_CLIPPING_PLANES 0\n");
    fs.push_back(u8"#version 300 es\n");
    fs.push_back(u8"precision mediump float;\n");
    fs.push_back(u8"#define THREE_VRM_THREE_REVISION 150\n");
    fs.push_back(u8"#define NUM_SPOT_LIGHT_COORDS 4\n");
    fs.push_back(u8"#define NUM_DIR_LIGHTS 0\n");
    fs.push_back(u8"#define NUM_POINT_LIGHTS 0\n");
    fs.push_back(u8"#define NUM_SPOT_LIGHTS 0\n");
    fs.push_back(u8"#define NUM_RECT_AREA_LIGHTS 0\n");
    fs.push_back(u8"#define NUM_HEMI_LIGHTS 0\n");
    fs.push_back(u8"#define NUM_SPOT_LIGHT_MAPS 0\n");
    fs.push_back(u8"#define NUM_CLIPPING_PLANES 0\n");
    fs.push_back(u8"#define UNION_CLIPPING_PLANES 0\n");
    fs.push_back(u8"#define isOrthographic false\n");
    fs.push_back(u8R"(
vec4 LinearToLinear( in vec4 value ) {
    return value;
}
vec4 GammaToLinear( in vec4 value, in float gammaFactor ) {
    return vec4( pow( value.xyz, vec3( gammaFactor ) ), value.w );
}
vec4 LinearToGamma( in vec4 value, in float gammaFactor ) {
    return vec4( pow( value.xyz, vec3( 1.0 / gammaFactor ) ), value.w );
}
vec4 sRGBToLinear( in vec4 value ) {
    return vec4( mix( pow( value.rgb * 0.9478672986 + vec3( 0.0521327014 ), vec3( 2.4 ) ), value.rgb * 0.0773993808, vec3( lessThanEqual( value.rgb, vec3( 0.04045 ) ) ) ), value.w );
}
vec4 LinearTosRGB( in vec4 value ) {
    return vec4( mix( pow( value.rgb, vec3( 0.41666 ) ) * 1.055 - vec3( 0.055 ), value.rgb * 12.92, vec3( lessThanEqual( value.rgb, vec3( 0.0031308 ) ) ) ), value.w );
}
vec4 RGBEToLinear( in vec4 value ) {
    return vec4( value.rgb * exp2( value.a * 255.0 - 128.0 ), 1.0 );
}
vec4 LinearToRGBE( in vec4 value ) {
    float maxComponent = max( max( value.r, value.g ), value.b );
    float fExp = clamp( ceil( log2( maxComponent ) ), -128.0, 127.0 );
    return vec4( value.rgb / exp2( fExp ), ( fExp + 128.0 ) / 255.0 );
}
vec4 RGBMToLinear( in vec4 value, in float maxRange ) {
    return vec4( value.xyz * value.w * maxRange, 1.0 );
}
vec4 LinearToRGBM( in vec4 value, in float maxRange ) {
    float maxRGB = max( value.x, max( value.g, value.b ) );
    float M      = clamp( maxRGB / maxRange, 0.0, 1.0 );
    M            = ceil( M * 255.0 ) / 255.0;
    return vec4( value.rgb / ( M * maxRange ), M );
}
vec4 RGBDToLinear( in vec4 value, in float maxRange ) {
    return vec4( value.rgb * ( ( maxRange / 255.0 ) / value.a ), 1.0 );
}
vec4 LinearToRGBD( in vec4 value, in float maxRange ) {
    float maxRGB = max( value.x, max( value.g, value.b ) );
    float D      = max( maxRange / maxRGB, 1.0 );
    D            = min( floor( D ) / 255.0, 1.0 );
    return vec4( value.rgb * ( D * ( 255.0 / maxRange ) ), D );
}
const mat3 cLogLuvM = mat3( 0.2209, 0.3390, 0.4184, 0.1138, 0.6780, 0.7319, 0.0102, 0.1130, 0.2969 );
vec4 LinearToLogLuv( in vec4 value ) {
    vec3 Xp_Y_XYZp = value.rgb * cLogLuvM;
    Xp_Y_XYZp = max(Xp_Y_XYZp, vec3(1e-6, 1e-6, 1e-6));
    vec4 vResult;
    vResult.xy = Xp_Y_XYZp.xy / Xp_Y_XYZp.z;
    float Le = 2.0 * log2(Xp_Y_XYZp.y) + 127.0;
    vResult.w = fract(Le);
    vResult.z = (Le - (floor(vResult.w*255.0))/255.0)/255.0;
    return vResult;
}
const mat3 cLogLuvInverseM = mat3( 6.0014, -2.7008, -1.7996, -1.3320, 3.1029, -5.7721, 0.3008, -1.0882, 5.6268 );
vec4 LogLuvToLinear( in vec4 value ) {
    float Le = value.z * 255.0 + value.w;
    vec3 Xp_Y_XYZp;
    Xp_Y_XYZp.y = exp2((Le - 127.0) / 2.0);
    Xp_Y_XYZp.z = Xp_Y_XYZp.y / value.y;
    Xp_Y_XYZp.x = value.x * Xp_Y_XYZp.z;
    vec3 vRGB = Xp_Y_XYZp.rgb * cLogLuvInverseM;
    return vec4( max(vRGB, 0.0), 1.0 );
}

vec4 mapTexelToLinear( vec4 value ) { return LinearToLinear( value ); }
vec4 envMapTexelToLinear( vec4 value ) { return LinearToLinear( value ); }
vec4 emissiveMapTexelToLinear( vec4 value ) { return LinearToLinear( value ); }
vec4 linearToOutputTexel( vec4 value ) { return LinearToLinear( value ); }
    )");
    vs.push_back(u8R"(
// = object.matrixWorld
uniform mat4 modelMatrix;

// = camera.matrixWorldInverse * object.matrixWorld
uniform mat4 modelViewMatrix;

// = camera.projectionMatrix
uniform mat4 projectionMatrix;

// = camera.matrixWorldInverse
uniform mat4 viewMatrix;

// = inverse transpose of modelViewMatrix
uniform mat3 normalMatrix;

// = camera position in world space
uniform vec3 cameraPosition;

// default vertex attributes provided by BufferGeometry
attribute vec3 position;
attribute vec3 normal;
attribute vec2 uv;

#ifdef USE_TANGENT
	attribute vec4 tangent;
#endif
#if defined( USE_COLOR_ALPHA )
	// vertex color attribute with alpha
	attribute vec4 color;
#elif defined( USE_COLOR )
	// vertex color attribute
	attribute vec3 color;
#endif

#ifdef USE_MORPHTARGETS

	attribute vec3 morphTarget0;
	attribute vec3 morphTarget1;
	attribute vec3 morphTarget2;
	attribute vec3 morphTarget3;

	#ifdef USE_MORPHNORMALS

		attribute vec3 morphNormal0;
		attribute vec3 morphNormal1;
		attribute vec3 morphNormal2;
		attribute vec3 morphNormal3;

	#else

		attribute vec3 morphTarget4;
		attribute vec3 morphTarget5;
		attribute vec3 morphTarget6;
		attribute vec3 morphTarget7;

	#endif
#endif
#ifdef USE_SKINNING
	attribute vec4 skinIndex;
	attribute vec4 skinWeight;
#endif
#ifdef USE_INSTANCING
	// Note that modelViewMatrix is not set when rendering an instanced model,
	// but can be calculated from viewMatrix * modelMatrix.
	//
	// Basic Usage:
	//   gl_Position = projectionMatrix * viewMatrix * modelMatrix * instanceMatrix * vec4(position, 1.0);
	attribute mat4 instanceMatrix;
#endif
    )");

    fs.push_back(u8R"(
uniform mat4 viewMatrix;
uniform vec3 cameraPosition;
    )");
    auto expanded = m_shaderSource.Get(ShaderTypes::MToon0);
    vs.push_back(expanded.Vert);
    fs.push_back(expanded.Frag);
    if (auto shader = grapho::gl3::ShaderProgram::Create(vs, fs)) {
      auto material = std::make_shared<grapho::gl3::Material>();
      material->Shader = *shader;
      // if (auto pbr = src.PbrMetallicRoughness()) {
      //   if (auto baseColorTexture = pbr->BaseColorTexture()) {
      //     if (auto texture =
      //           GetOrCreateTexture(root,
      //                              bin,
      //                              baseColorTexture->Index(),
      //                              libvrm::gltf::ColorSpace::sRGB)) {
      //       material->Textures.push_back({ 0, texture });
      //     }
      //   }
      // }
      return material;
    } else {
      // App::Instance().Log(LogLevel::Error)
      //   << "[VS]" << gltfjson::tree::from_u8(vs.back());
      // App::Instance().Log(LogLevel::Error)
      //   << "[FS]" << gltfjson::tree::from_u8(fs.back());
      App::Instance().Log(LogLevel::Error) << shader.error();
    }

    return {};
  }

  std::shared_ptr<grapho::gl3::Material> CreateMaterialMToon1(
    const gltfjson::typing::Root& root,
    const gltfjson::typing::Bin& bin,
    const gltfjson::typing::Material& src

  )
  {
    App::Instance().Log(LogLevel::Error) << "vrm1 not implemented";
    return {};
  }

  std::shared_ptr<grapho::gl3::Material> CreateMaterialUnlit(
    const gltfjson::typing::Root& root,
    const gltfjson::typing::Bin& bin,
    std::optional<uint32_t> id,
    const gltfjson::typing::Material& src)
  {
    std::vector<std::u8string_view> vs;
    std::vector<std::u8string_view> fs;
    vs.push_back(u8"#version 450\n");
    fs.push_back(u8"#version 450\n");
    if (GetAlphaMode(root, id) == gltfjson::format::AlphaModes::Mask) {
      fs.push_back(u8"#define MODE_MASK\n");
    }

    auto expanded = m_shaderSource.Get(ShaderTypes::Unlit);
    vs.push_back(expanded.Vert);
    fs.push_back(expanded.Frag);
    if (auto shader = grapho::gl3::ShaderProgram::Create(vs, fs)) {
      auto material = std::make_shared<grapho::gl3::Material>();
      material->Shader = *shader;
      if (auto pbr = src.PbrMetallicRoughness()) {
        if (auto baseColorTexture = pbr->BaseColorTexture()) {
          if (auto texture =
                GetOrCreateTexture(root,
                                   bin,
                                   baseColorTexture->Index(),
                                   libvrm::gltf::ColorSpace::sRGB)) {
            material->Textures.push_back({ 0, texture });
          }
        }
      }
      return material;
    } else {
      App::Instance().Log(LogLevel::Error) << shader.error();
    }

    return {};
  }

  std::shared_ptr<grapho::gl3::Material> CreateMaterialPbr(
    const gltfjson::typing::Root& root,
    const gltfjson::typing::Bin& bin,
    const gltfjson::typing::Material& src,
    bool isUnlit)
  {
    std::shared_ptr<grapho::gl3::Texture> albedo;
    std::shared_ptr<grapho::gl3::Texture> metallic;
    std::shared_ptr<grapho::gl3::Texture> roughness;
    if (auto pbr = src.PbrMetallicRoughness()) {
      if (auto baseColorTexture = pbr->BaseColorTexture()) {
        albedo = GetOrCreateTexture(
          root, bin, baseColorTexture->Index(), libvrm::gltf::ColorSpace::sRGB);
      }
      if (auto metallicRoughnessTexture = pbr->MetallicRoughnessTexture()) {
        metallic = GetOrCreateTexture(root,
                                      bin,
                                      metallicRoughnessTexture->Index(),
                                      libvrm::gltf::ColorSpace::Linear);
        roughness = GetOrCreateTexture(root,
                                       bin,
                                       metallicRoughnessTexture->Index(),
                                       libvrm::gltf::ColorSpace::Linear);
      }
    }
    std::shared_ptr<grapho::gl3::Texture> normal;
    if (auto normalTexture = src.NormalTexture()) {
      normal = GetOrCreateTexture(
        root, bin, normalTexture->Index(), libvrm::gltf::ColorSpace::Linear);
    }
    std::shared_ptr<grapho::gl3::Texture> ao;
    if (auto occlusionTexture = src.OcclusionTexture()) {
      ao = GetOrCreateTexture(
        root, bin, occlusionTexture->Index(), libvrm::gltf::ColorSpace::Linear);
    }

    std::vector<std::u8string_view> vs;
    std::vector<std::u8string_view> fs;
    vs.push_back(u8"#version 300 es\n");
    fs.push_back(u8"#version 300 es\n");
    if (albedo) {
      fs.push_back(u8"#define HAS_ALBEDO_TEXTURE\n");
    }
    if (metallic) {
      fs.push_back(u8"#define HAS_METALLIC_TEXTURE\n");
    }
    if (roughness) {
      fs.push_back(u8"#define HAS_ROUGHNESS_TEXTURE\n");
    }
    if (ao) {
      fs.push_back(u8"#define HAS_AO_TEXTURE\n");
    }
    if (normal) {
      fs.push_back(u8"#define HAS_NORMAL_TEXTURE\n");
    }
    auto expanded = m_shaderSource.Get(ShaderTypes::Pbr);
    vs.push_back(expanded.Vert);
    fs.push_back(expanded.Frag);
    if (auto material = grapho::gl3::CreatePbrMaterial(
          albedo, normal, metallic, roughness, ao, vs, fs)) {
      return *material;
    } else {
      App::Instance().Log(LogLevel::Error)
        << "CreatePbrMaterial: " << material.error();
    }

    return nullptr;
  }

  std::shared_ptr<grapho::gl3::Material> GetOrCreateMaterial(
    const gltfjson::typing::Root& root,
    const gltfjson::typing::Bin& bin,
    std::optional<uint32_t> id)
  {
    if (!id) {
      return {};
    }

    auto found = m_materialMap.find(*id);
    if (found != m_materialMap.end()) {
      return found->second;
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

    std::shared_ptr<grapho::gl3::Material> material;
    if (mtoon1) {
      material = CreateMaterialMToon1(root, bin, src);
    } else if (mtoon0) {
      material = CreateMaterialMToon0(root, bin, mtoon0);
    }
    // else if (unlit) {
    //   material = CreateMaterialUnlit(root, bin, id, src);
    // }
    else {
      material = CreateMaterialPbr(root, bin, src, unlit != nullptr);
    }

    if (material) {
      auto inserted = m_materialMap.insert({ *id, material });
      return inserted.first->second;
    } else {
      m_materialMap.insert({ *id, {} });
      return {};
    }
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

    switch (pass) {
      case RenderPass::Color: {
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
        m_model.model = m;
        m_model.CalcNormalMatrix();
        uint32_t drawOffset = 0;
        for (auto& primitive : mesh->m_primitives) {
          DrawPrimitive(env.ProjectionMatrix,
                        env.ViewMatrix,
                        m,
                        env.CameraPosition,
                        root,
                        bin,
                        vao,
                        primitive,
                        drawOffset);
          drawOffset += primitive.DrawCount * 4;
        }
        break;
      }

      case RenderPass::ShadowMatrix: {
        if (!m_shadow) {
          auto expanded = m_shaderSource.Get(ShaderTypes::Shadow);
          if (auto shadow = grapho::gl3::ShaderProgram::Create(expanded.Vert,
                                                               expanded.Frag)) {
            m_shadow = *shadow;
          } else {
            App::Instance().Log(LogLevel::Error) << shadow.error();
          }
        }
        if (m_shadow) {
          m_shadow->Use();
          m_shadow->Uniform("Projection")->SetMat4(env.ProjectionMatrix);
          m_shadow->Uniform("View")->SetMat4(env.ViewMatrix);
          m_shadow->Uniform("Shadow")->SetMat4(env.ShadowMatrix);
          m_shadow->Uniform("Model")->SetMat4(m);
          uint32_t drawCount = 0;
          for (auto& primitive : mesh->m_primitives) {
            drawCount += primitive.DrawCount * 4;
          }
          vao->Draw(GL_TRIANGLES, drawCount, 0);
        }
        break;
      }
    }
  }

  void DrawPrimitive(const DirectX::XMFLOAT4X4& projection,
                     const DirectX::XMFLOAT4X4& view,
                     const DirectX::XMFLOAT4X4& model,
                     const DirectX::XMFLOAT3& cameraPos,
                     const gltfjson::typing::Root& root,
                     const gltfjson::typing::Bin& bin,
                     const std::shared_ptr<grapho::gl3::Vao>& vao,
                     const runtimescene::Primitive& primitive,
                     uint32_t drawOffset)
  {
    auto material = GetOrCreateMaterial(root, bin, primitive.Material);
    if (material) {
      // update ubo
      auto gltfMaterial = root.Materials[*primitive.Material];
      if (auto cutoff = gltfMaterial.AlphaCutoff()) {
        m_model.cutoff.x = *cutoff;
      }
      m_model.color = { 1, 1, 1, 1 };
      if (auto pbr = gltfMaterial.PbrMetallicRoughness()) {
        if (pbr->BaseColorFactor.size() == 4) {
          m_model.color.x = pbr->BaseColorFactor[0];
          m_model.color.y = pbr->BaseColorFactor[1];
          m_model.color.z = pbr->BaseColorFactor[2];
          m_model.color.w = pbr->BaseColorFactor[3];
        }
      }
      m_modelUbo->Upload(m_model);
      m_modelUbo->SetBindingPoint(1);

      material->Activate();

      // state
      glEnable(GL_CULL_FACE);
      glFrontFace(GL_CCW);
      glCullFace(GL_BGR);
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
      if (!m_error) {
        auto expanded = m_shaderSource.Get(ShaderTypes::Error);
        if (auto error = grapho::gl3::ShaderProgram::Create(expanded.Vert,
                                                            expanded.Frag)) {
          m_error = *error;
        }
      }
      if (m_error) {
        m_error->Use();
        m_error->Uniform("Projection")->SetMat4(projection);
        m_error->Uniform("View")->SetMat4(view);
        m_error->Uniform("Model")->SetMat4(model);
      }
    }
    vao->Draw(GL_TRIANGLES, primitive.DrawCount, drawOffset);
  }

  void CreateDock(const AddDockFunc& addDock, std::string_view title)
  {
    addDock(grapho::imgui::Dock(title, [this]() {
      ImGui::Text("srgb: %zd(textures)", m_srgbTextureMap.size());
      ImGui::Text("linear: %zd(textures)", m_linearTextureMap.size());
      ImGui::SameLine();
      ImGui::Text("%zd(meshes)", m_drawableMap.size());
    }));
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
CreateDock(const AddDockFunc& addDock, std::string_view title)
{
  Gl3Renderer::Instance().CreateDock(addDock, title);
}

std::shared_ptr<grapho::gl3::Texture>
GetOrCreateTexture(const gltfjson::typing::Root& root,
                   const gltfjson::typing::Bin& bin,
                   std::optional<uint32_t> texture,
                   libvrm::gltf::ColorSpace colorspace)
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
