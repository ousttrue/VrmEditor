#include "vrm/bvhresolver.h"
#include "vrm/node.h"

namespace bvh {
// [x, y, z][c6][c5][c4][c3][c2][c1][parent][root]
static void
ResolveFrame(const std::shared_ptr<gltf::Scene>& scene,
             std::shared_ptr<gltf::Node>& node,
             const std::shared_ptr<bvh::Bvh>& bvh,
             const bvh::Frame& frame
             // DirectX::XMMATRIX m,
             // float scaling
             // std::span<DirectX::XMFLOAT4X4>::iterator& out,
             // std::span<DirectX::XMFLOAT4>::iterator& outLocal
)
{
  auto joint = &bvh->joints[scene->GetNodeIndex(node)];
  auto transform = frame.Resolve(joint->channels);

  // auto t = DirectX::XMMatrixTranslation(transform.Translation.x * scaling,
  //                                       transform.Translation.y * scaling,
  //                                       transform.Translation.z * scaling);

  node->Transform.Translation = transform.Translation;

  // auto r =
  //     DirectX::XMLoadFloat3x3((const DirectX::XMFLOAT3X3
  //     *)&transform.Rotation);
  // auto r = DirectX::XMMatrixRotationQuaternion(
  //   DirectX::XMLoadFloat4(&transform.Rotation));

  node->Transform.Rotation = transform.Rotation;

  // DirectX::XMStoreFloat4(&*outLocal, DirectX::XMQuaternionRotationMatrix(r));
  // *outLocal = transform.Rotation;

  // auto local = r * t;

  // m = local * m;
  // auto shape = DirectX::XMLoadFloat4x4(&node->ShapeMatrix);
  // DirectX::XMStoreFloat4x4(&*out, shape * m);
  // ++out;
  // ++outLocal;
  for (auto& child : node->Children) {
    ResolveFrame(scene, child, bvh, frame);
  }
}

void
ResolveFrame(const std::shared_ptr<gltf::Scene>& scene,
             const std::shared_ptr<bvh::Bvh>& bvh,
             Time time)
{
  auto index = bvh->TimeToIndex(time);
  auto frame = bvh->GetFrame(index);
  // auto span = std::span(cuber->Instances);
  // auto it = span.begin();
  // auto t_span = std::span(scene->LocalRotations);
  // auto t = t_span.begin();
  ResolveFrame(scene, scene->m_roots[0], bvh, frame
               // DirectX::XMMatrixIdentity(),
               // bvh->GuessScaling(),
               // it,
               // t
  );
}

}
