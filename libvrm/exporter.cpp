#include "vrm/exporter.h"
#include "vrm/gltf.h"
#include "vrm/jsons.h"
#include "vrm/runtimescene/base_mesh.h"
#include "vrm/scenetypes.h"
#include <gltfjson/glb.h>
#include <gltfjson/gltf.h>
#include <gltfjson/gltf_types.h>

struct Context
{
  libvrm::jsons::Writer& m_writer;
  gltfjson::format::BinWriter& m_binWriter;
};

namespace gltfjson {
namespace format {

template<>
inline Accessor
CreateAccessor<runtimescene::ushort4>()
{
  return Accessor{
    .ComponentType = ComponentTypes::UNSIGNED_SHORT,
    .Type = Types::VEC4,
  };
}
template<>
inline Accessor
CreateAccessor<DirectX::XMFLOAT2>()
{
  return Accessor{
    .ComponentType = ComponentTypes::FLOAT,
    .Type = Types::VEC2,
  };
}
template<>
inline Accessor
CreateAccessor<DirectX::XMFLOAT3>()
{
  return Accessor{
    .ComponentType = ComponentTypes::FLOAT,
    .Type = Types::VEC3,
  };
}
template<>
inline Accessor
CreateAccessor<DirectX::XMFLOAT4>()
{
  return Accessor{
    .ComponentType = ComponentTypes::FLOAT,
    .Type = Types::VEC4,
  };
}
template<>
inline Accessor
CreateAccessor<DirectX::XMFLOAT4X4>()
{
  return Accessor{
    .ComponentType = ComponentTypes::FLOAT,
    .Type = Types::MAT4,
  };
}

}
}

namespace libvrm::gltf {

Exporter::Exporter()
  : m_writer([this](std::string_view str) {
    auto size = JsonChunk.size();
    JsonChunk.resize(size + str.size());
    std::copy((const uint8_t*)str.data(),
              (const uint8_t*)str.data() + str.size(),
              JsonChunk.data() + size);
    ;
  })
  , m_binWriter([this](std::span<const uint8_t> values) {
    auto size = BinChunk.size();
    BinChunk.resize(size + values.size());
    std::copy(
      values.data(), values.data() + values.size(), BinChunk.data() + size);
  })
{
}

// "asset": {
//     "version": "2.0",
//     "generator": "collada2gltf@f356b99aef8868f74877c7ca545f2cd206b9d3b7",
//     "copyright": "2017 (c) Khronos Group"
// }
static void
ExportAsset(Context& c, const GltfRoot& scene)
{
  c.m_writer.object_open();

  c.m_writer.key("version");
  c.m_writer.value("2.0");

  c.m_writer.key("generator");
  c.m_writer.value("VrmEditor");

  c.m_writer.object_close();
}

// {
//     "bufferView": 14,
//     "mimeType": "image/jpeg"
// }
static void
ExportImage(Context& c,
            const GltfRoot& scene,
            const gltfjson::format::Image& image)
{
  c.m_writer.object_open();
  c.m_writer.key("name");
  c.m_writer.value(image.Name);
  // if (auto encoded = image->Encoded) {
  //   switch (encoded->Type) {
  //     case ImageType::Jpeg:
  //       m_writer.key("mimeType");
  //       m_writer.value("image/jpeg");
  //       break;
  //     case ImageType::Png:
  //       m_writer.key("mimeType");
  //       m_writer.value("image/png");
  //       break;
  //     default:
  //       assert(false);
  //       break;
  //   }
  //   auto index = m_binWriter.PushBufferView(encoded->Bytes);
  //   m_writer.key("bufferView");
  //   m_writer.value(index);
  // } else {
  //   // encode to png ?
  //   assert(false);
  // }
  c.m_writer.object_close();
}

// {
//     "magFilter": 9729,
//     "minFilter": 9987,
//     "wrapS": 10497,
//     "wrapT": 10497
// }
static void
ExportTextureSampler(Context& c,
                     const GltfRoot& scene,
                     const gltfjson::format::Sampler& sampler)
{
  c.m_writer.object_open();
  c.m_writer.object_close();
}

// {
//     "sampler": 0,
//     "source": 2
// }
static void
ExportTexture(Context& c,
              const GltfRoot& scene,
              const gltfjson::format::Texture& texture)
{
  c.m_writer.object_open();
  if (auto source = texture.Source) {
    c.m_writer.key("source");
    c.m_writer.value(*source);
  }
  if (auto sampler = texture.Sampler) {
    c.m_writer.key("sampler");
    c.m_writer.value(*sampler);
  }
  c.m_writer.object_close();
}

// {
//     "name": "gold",
//     "pbrMetallicRoughness": {
//         "baseColorFactor": [ 1.000, 0.766, 0.336, 1.0 ],
//         "baseColorTexture": {
//             "index": 0,
//             "texCoord": 1
//         },
//         "metallicFactor": 1.0,
//         "roughnessFactor": 0.0
//     }
// }
static void
ExportMaterial(Context& c,
               const GltfRoot& scene,
               const gltfjson::format::Material& material)
{
  c.m_writer.object_open();
  c.m_writer.key("name");
  c.m_writer.value(material.Name);
  c.m_writer.key("pbrMetallicRoughness");
  {
    c.m_writer.object_open();
    // if (material->ColorTexture) {
    //   c.m_writer.key("baseColorTexture");
    //   {
    //     c.m_writer.object_open();
    //     c.m_writer.key("index");
    //     c.m_writer.value(*scene.IndexOf(material->ColorTexture));
    //     c.m_writer.object_close();
    //   }
    // }
    c.m_writer.object_close();
  }
  c.m_writer.object_close();
}

template<typename T, typename V, typename S>
inline void
PushVertices(std::vector<T>& indices,
             const std::shared_ptr<runtimescene::BaseMesh>& mesh,
             uint32_t& index,
             uint32_t drawCount,
             const V& push_vertex,
             bool has_skinning,
             const S& push_skinning)
{
  std::unordered_map<uint32_t, uint32_t> vertexIndexMap;
  for (int i = 0; i < drawCount; ++i, ++index) {
    auto vertex_index = mesh->m_indices[index];
    auto found = vertexIndexMap.find(vertex_index);
    if (found == vertexIndexMap.end()) {
      // new vertex
      // 分割するので index が 0 に戻るので vertex_index ではなく i を使う
      indices.push_back(i);
      vertexIndexMap.insert({ vertex_index, i });
    } else {
      // already exists
      indices.push_back(found->second);
    }
    push_vertex(mesh->m_vertices[vertex_index]);
    if (has_skinning) {
      push_skinning(mesh->m_bindings[vertex_index]);
    }
  }
}

//
// vertex buffer を submesh で分割する
//
static uint32_t
ExportMeshPrimitive(Context& c,
                    const std::shared_ptr<runtimescene::BaseMesh>& mesh,
                    const runtimescene::Primitive& prim,
                    uint32_t index)
{
  c.m_writer.object_open();

  if (auto material = prim.Material) {
    c.m_writer.key("material");
    c.m_writer.value(*material);
  }

  std::vector<DirectX::XMFLOAT3> positions;
  std::vector<DirectX::XMFLOAT3> normals;
  std::vector<DirectX::XMFLOAT2> tex0;
  auto push_vertex =
    [&positions, &normals, &tex0](const runtimescene::Vertex& v) {
      positions.push_back(v.Position);
      normals.push_back(v.Normal);
      tex0.push_back(v.Uv);
    };

  std::vector<runtimescene::ushort4> joints;
  std::vector<DirectX::XMFLOAT4> weights;
  auto push_skinning = [&joints,
                        &weights](const runtimescene::JointBinding& v) {
    joints.push_back(v.Joints);
    weights.push_back(v.Weights);
  };
  bool has_skinning = mesh->m_bindings.size() == mesh->m_vertices.size();

  if (mesh->m_vertices.size() < 255) {
    std::vector<uint8_t> indices;
    PushVertices(indices,
                 mesh,
                 index,
                 prim.DrawCount,
                 push_vertex,
                 has_skinning,
                 push_skinning);
    auto indices_index = c.m_binWriter.PushAccessor(indices);
    c.m_writer.key("indices");
    c.m_writer.value(indices_index);

  } else if (mesh->m_vertices.size() < 65535) {
    std::vector<uint16_t> indices;
    PushVertices(indices,
                 mesh,
                 index,
                 prim.DrawCount,
                 push_vertex,
                 has_skinning,
                 push_skinning);
    auto indices_index = c.m_binWriter.PushAccessor(indices);
    c.m_writer.key("indices");
    c.m_writer.value(indices_index);

  } else {
    std::vector<uint32_t> indices;
    PushVertices(indices,
                 mesh,
                 index,
                 prim.DrawCount,
                 push_vertex,
                 has_skinning,
                 push_skinning);
    auto indices_index = c.m_binWriter.PushAccessor(indices);
    c.m_writer.key("indices");
    c.m_writer.value(indices_index);
  }

  c.m_writer.key("attributes");
  {
    c.m_writer.object_open();
    {
      auto position_accessor_index = c.m_binWriter.PushAccessor(positions);
      c.m_writer.key(gltfjson::format::VERTEX_POSITION);
      c.m_writer.value(position_accessor_index);
    }
    {
      auto normal_accessor_index = c.m_binWriter.PushAccessor(normals);
      c.m_writer.key(gltfjson::format::VERTEX_NORMAL);
      c.m_writer.value(normal_accessor_index);
    }
    {
      auto tex0_accessor_index = c.m_binWriter.PushAccessor(tex0);
      c.m_writer.key(gltfjson::format::VERTEX_UV);
      c.m_writer.value(tex0_accessor_index);
    }
    // skinning
    if (has_skinning) {
      auto joint_accessor_index = c.m_binWriter.PushAccessor(joints);
      c.m_writer.key(gltfjson::format::VERTEX_JOINT);
      c.m_writer.value(joint_accessor_index);
      auto weight_accessor_index = c.m_binWriter.PushAccessor(weights);
      c.m_writer.key(gltfjson::format::VERTEX_WEIGHT);
      c.m_writer.value(weight_accessor_index);
    }
    c.m_writer.object_close();
  }

  c.m_writer.object_close();

  return index;
}

// {
//     "primitives": [
//         {
//             "attributes": {
//                 "NORMAL": 23,
//                 "POSITION": 22,
//                 "TANGENT": 24,
//                 "TEXCOORD_0": 25
//             },
//             "indices": 21,
//             "material": 3,
//             "mode": 4
//         }
//     ]
// }
static void
ExportMesh(Context& c, const gltfjson::format::Mesh& mesh)
{
  c.m_writer.object_open();
  if (mesh.Name.size()) {
    c.m_writer.key("name");
    c.m_writer.value(mesh.Name);
  }
  c.m_writer.key("primitives");
  c.m_writer.array_open();
  // uint32_t index = 0;
  // for (auto& prim : mesh.Primitives) {
  //   index = ExportMeshPrimitive(c, mesh, prim, index);
  // }
  c.m_writer.array_close();
  c.m_writer.object_close();
}

// {
//     "inverseBindMatrices": 0,
//     "joints": [ 1, 2 ],
//     "skeleton": 1
// }
static void
ExportSkin(Context& c, const GltfRoot& scene, const std::shared_ptr<Skin>& skin)
{
  c.m_writer.object_open();
  c.m_writer.key("joints");
  {
    c.m_writer.array_open();
    for (auto joint : skin->Joints) {
      c.m_writer.value(joint);
    }
    c.m_writer.array_close();
  }
  {
    auto accessor_index = c.m_binWriter.PushAccessor(skin->BindMatrices);
    c.m_writer.key("inverseBindMatrices");
    c.m_writer.value(accessor_index);
  }
  c.m_writer.object_close();
}

static void
ExportNode(Context& c, const GltfRoot& scene, const std::shared_ptr<Node>& node)
{
  c.m_writer.object_open();
  c.m_writer.key("name");
  c.m_writer.value(node->Name);
  // TRS
  if (node->InitialTransform.Translation.x != 0 ||
      node->InitialTransform.Translation.y != 0 ||
      node->InitialTransform.Translation.z != 0) {
    c.m_writer.key("translation");
    c.m_writer.array_open();
    c.m_writer.value(node->InitialTransform.Translation.x);
    c.m_writer.value(node->InitialTransform.Translation.y);
    c.m_writer.value(node->InitialTransform.Translation.z);
    c.m_writer.array_close();
  }
  if (node->Children.size()) {
    c.m_writer.key("children");
    c.m_writer.array_open();
    for (auto& child : node->Children) {
      if (auto i = scene.IndexOf(child)) {
        c.m_writer.value(*i);
      }
    }
    c.m_writer.array_close();
  }
  if (node->Mesh) {
    c.m_writer.key("mesh");
    c.m_writer.value(*node->Mesh);
  }
  if (node->Skin) {
    c.m_writer.key("skin");
    c.m_writer.value(*scene.IndexOf(node->Skin));
  }
  c.m_writer.object_close();
}

static void
ExportBuffersViewsAccessors(Context& c, const GltfRoot& scene)
{
  if (c.m_binWriter.Accessors.size()) {
    c.m_writer.key("accessors");
    {
      c.m_writer.array_open();
      for (auto& accessor : c.m_binWriter.Accessors) {
        c.m_writer.object_open();
        c.m_writer.key("bufferView");
        c.m_writer.value(*accessor.BufferView);
        c.m_writer.key("count");
        c.m_writer.value(accessor.Count);
        c.m_writer.key("byteOffset");
        c.m_writer.value(0);
        c.m_writer.key("type");
        c.m_writer.value(gltfjson::format::type_str(accessor.Type));
        c.m_writer.key("componentType");
        c.m_writer.value((int)accessor.ComponentType);
        c.m_writer.object_close();
      }
      c.m_writer.array_close();
    }
  }
  if (c.m_binWriter.BufferViews.size()) {
    c.m_writer.key("bufferViews");
    {
      c.m_writer.array_open();
      for (auto& bufferView : c.m_binWriter.BufferViews) {
        c.m_writer.object_open();
        c.m_writer.key("buffer");
        c.m_writer.value(0); // for glb
        c.m_writer.key("byteOffset");
        c.m_writer.value(bufferView.ByteOffset);
        c.m_writer.key("byteLength");
        c.m_writer.value(bufferView.ByteLength);
        c.m_writer.object_close();
      }
      c.m_writer.array_close();
    }

    c.m_writer.key("buffers");
    {
      c.m_writer.array_open();
      {
        c.m_writer.object_open();
        c.m_writer.key("byteLength");
        c.m_writer.value(c.m_binWriter.BufferPosition);
        c.m_writer.object_close();
      }
      c.m_writer.array_close();
    }
  }
}

struct AnimationSampler
{
  uint32_t Input;
  uint32_t Output;
  AnimationInterpolationModes Interpolation;
};

struct AnimationChannel
{
  uint32_t Sampler;
  uint32_t Node;
  AnimationTargets Target;
};

struct AnimationExporter
{
  gltfjson::format::BinWriter& m_binWriter;
  std::vector<AnimationSampler> AnimationSamplers;
  std::vector<AnimationChannel> AnimationChannels;

  AnimationExporter(gltfjson::format::BinWriter& binWriter)
    : m_binWriter(binWriter)
  {
  }

  template<typename T>
  uint32_t ExportAnimationSampler(
    std::span<const float> times,
    std::span<const T> values,
    AnimationInterpolationModes mode = AnimationInterpolationModes::LINEAR)
  {
    auto index = AnimationSamplers.size();
    auto input_index = m_binWriter.PushAccessor(times);
    auto output_index = m_binWriter.PushAccessor(values);

    AnimationSamplers.push_back({
      .Input = input_index,
      .Output = output_index,
      .Interpolation = mode,
    });

    return index;
  }

  void ExportAnimationTranslation(uint32_t node,
                                  const Curve<DirectX::XMFLOAT3>& curve)
  {
    auto sampler =
      ExportAnimationSampler<DirectX::XMFLOAT3>(curve.Times, curve.Values);
    ExportAnimationChannel(sampler, node, AnimationTargets::TRANSLATION);
  }

  void ExportAnimationRotation(uint32_t node,
                               const Curve<DirectX::XMFLOAT4>& curve)
  {
    auto sampler =
      ExportAnimationSampler<DirectX::XMFLOAT4>(curve.Times, curve.Values);
    ExportAnimationChannel(sampler, node, AnimationTargets::ROTATION);
  }

  void ExportAnimationScale(uint32_t node,
                            const Curve<DirectX::XMFLOAT3>& curve)
  {
    auto sampler =
      ExportAnimationSampler<DirectX::XMFLOAT3>(curve.Times, curve.Values);
    ExportAnimationChannel(sampler, node, AnimationTargets::SCALE);
  }

  void ExportAnimationWeights(uint32_t node, const WeightsCurve& curve)
  {
    // auto sampler =
    //   ExportAnimationSampler<DirectX::XMFLOAT3>(curve.Times, curve.Values);
    // ExportAnimationChannel(sampler, node, AnimationTargets::SCALE);
  }

  void ExportAnimationChannel(uint32_t sampler,
                              uint32_t node,
                              AnimationTargets target)
  {
    AnimationChannels.push_back({
      .Sampler = sampler,
      .Node = node,
      .Target = target,
    });
  }
};

static void
ExportAnimations(Context& c, const GltfRoot& scene)
{
  if (scene.m_animations.empty()) {
    return;
  }

  c.m_writer.key("animations");
  c.m_writer.array_open();

  // perpare
  for (auto& animation : scene.m_animations) {
    AnimationExporter animationExporter(c.m_binWriter);

    for (auto& [k, v] : animation->m_translationMap) {
      animationExporter.ExportAnimationTranslation(k, v);
    }
    for (auto& [k, v] : animation->m_rotationMap) {
      animationExporter.ExportAnimationRotation(k, v);
    }
    for (auto& [k, v] : animation->m_scaleMap) {
      animationExporter.ExportAnimationScale(k, v);
    }
    for (auto& [k, v] : animation->m_weightsMap) {
      animationExporter.ExportAnimationWeights(k, v);
    }

    c.m_writer.object_open();
    {
      c.m_writer.key("name");
      c.m_writer.value(animation->m_name);

      c.m_writer.key("samplers");
      c.m_writer.array_open();
      for (auto& sampler : animationExporter.AnimationSamplers) {
        c.m_writer.object_open();
        c.m_writer.key("input");
        c.m_writer.value(sampler.Input);
        c.m_writer.key("output");
        c.m_writer.value(sampler.Output);
        c.m_writer.key("interpolation");
        c.m_writer.value(
          AnimationInterpolationModeNames[(int)sampler.Interpolation]);
        c.m_writer.object_close();
      }
      c.m_writer.array_close();

      c.m_writer.key("channels");
      c.m_writer.array_open();
      for (auto& channel : animationExporter.AnimationChannels) {
        c.m_writer.object_open();
        c.m_writer.key("sampler");
        c.m_writer.value(channel.Sampler);
        c.m_writer.key("target");
        c.m_writer.object_open();
        c.m_writer.key("node");
        c.m_writer.value(channel.Node);
        c.m_writer.key("path");
        c.m_writer.value(AnimationTargetNames[(int)channel.Target]);
        c.m_writer.object_close();
        c.m_writer.object_close();
      }
      c.m_writer.array_close();
    }
    c.m_writer.object_close();
  }

  c.m_writer.array_close();
}

void
Exporter::Export(const GltfRoot& scene)
{
  m_writer.object_open();

  Context c{ m_writer, m_binWriter };
  m_writer.key("asset");
  {
    ExportAsset(c, scene);
  }

  // images
  if (scene.m_gltf.Images.Size()) {
    m_writer.key("images");
    {
      m_writer.array_open();
      for (auto& image : scene.m_gltf.Images) {
        ExportImage(c, scene, image);
      }
      m_writer.array_close();
    }
  }

  // samplers
  if (scene.m_gltf.Samplers.Size()) {
    m_writer.key("samplers");
    {
      m_writer.array_open();
      for (auto& sampler : scene.m_gltf.Samplers) {
        ExportTextureSampler(c, scene, sampler);
      }
      m_writer.array_close();
    }
  }

  // textures
  if (scene.m_gltf.Textures.Size()) {
    m_writer.key("textures");
    {
      m_writer.array_open();
      for (auto& texture : scene.m_gltf.Textures) {
        ExportTexture(c, scene, texture);
      }
      m_writer.array_close();
    }
  }

  // materials
  if (scene.m_gltf.Materials.Size()) {
    m_writer.key("materials");
    {
      m_writer.array_open();
      for (auto& material : scene.m_gltf.Materials) {
        ExportMaterial(c, scene, material);
      }
      m_writer.array_close();
    }
  }

  // meshes
  if (scene.m_gltf.Meshes.Size()) {
    m_writer.key("meshes");
    {
      m_writer.array_open();
      for (auto& mesh : scene.m_gltf.Meshes) {
        ExportMesh(c, mesh);
      }
      m_writer.array_close();
    }
  }

  // skins
  if (scene.m_skins.size()) {
    m_writer.key("skins");
    {
      m_writer.array_open();
      for (auto& skin : scene.m_skins) {
        ExportSkin(c, scene, skin);
      }
      m_writer.array_close();
    }
  }

  // nodes
  if (scene.m_nodes.size()) {
    m_writer.key("nodes");
    {
      m_writer.array_open();
      for (auto& node : scene.m_nodes) {
        ExportNode(c, scene, node);
      }
      m_writer.array_close();
    }
    m_writer.key("scenes");
    {
      m_writer.array_open();
      m_writer.object_open();
      m_writer.key("nodes");
      m_writer.array_open();
      for (auto& root : scene.m_roots) {
        m_writer.value(*scene.IndexOf(root));
      }
      m_writer.array_close();
      m_writer.object_close();
      m_writer.array_close();
    }
    m_writer.key("scene");
    {
      m_writer.value(0);
    }
  }

  // update bin
  ExportAnimations(c, scene);

  // last
  ExportBuffersViewsAccessors(c, scene);

  //
  // extensions
  //
  if (scene.m_vrma) {
    m_writer.key("extensions");
    m_writer.object_open();
    {
      m_writer.key("VRMC_vrm_animation");
      m_writer.object_open();
      {
        m_writer.key("expressions");
        m_writer.object_open();
        {
          m_writer.key("preset");
          m_writer.object_open();
          {
            m_writer.key("aa");
            m_writer.object_open();
            {
              m_writer.key("node");
              m_writer.value(0);
            }
            m_writer.object_close();
          }
          m_writer.object_close();
        }
        m_writer.object_close();
      }
      m_writer.object_close();
    }
    m_writer.object_close();
  }

  m_writer.object_close();
}

}
