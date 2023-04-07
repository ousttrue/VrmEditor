#include "cuber.h"
#include "viewporjection.h"
#include <cuber/gl3/GlCubeRenderer.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <vrm/scene.h>

Cuber::Cuber()
{
  m_cuber = std::make_shared<cuber::gl3::GlCubeRenderer>();
  m_liner = std::make_shared<cuber::gl3::GlLineRenderer>();
  cuber::PushGrid(m_lines);
}

const float DEFAULT_SIZE = 0.04f;

void
Cuber::CalcShape(const std::shared_ptr<gltf::Node>& node, float scaling)
{
  m_scaling = scaling;

  DirectX::XMStoreFloat4x4(
    &node->ShapeMatrix,
    DirectX::XMMatrixScaling(DEFAULT_SIZE, DEFAULT_SIZE, DEFAULT_SIZE));

  auto isRoot_ = Instances.empty();
  Instances.push_back({});
  if (!isRoot_) {
    std::shared_ptr<gltf::Node> tail;
    switch (node->Children.size()) {
      case 0:
        return;

      case 1:
        tail = node->Children.front();
        break;

      default:
        for (auto& child : node->Children) {
          if (!tail) {
            tail = child;
          } else if (std::abs(child->Transform.Translation.x) <
                     std::abs(tail->Transform.Translation.x)) {
            // coose center node
            tail = child;
          }
        }
    }

    auto _Y = DirectX::XMFLOAT3(tail->Transform.Translation.x * scaling,
                                tail->Transform.Translation.y * scaling,
                                tail->Transform.Translation.z * scaling);
    auto Y = DirectX::XMLoadFloat3(&_Y);

    auto length = DirectX::XMVectorGetX(DirectX::XMVector3Length(Y));
    // std::cout << name_ << "=>" << tail->name_ << "=" << length <<
    // std::endl;
    Y = DirectX::XMVector3Normalize(Y);
    auto _Z = DirectX::XMFLOAT3(0, 0, 1);
    auto Z = DirectX::XMLoadFloat3(&_Z);
    auto X = DirectX::XMVector3Cross(Y, Z);
    Z = DirectX::XMVector3Cross(X, Y);

    auto center = DirectX::XMMatrixTranslation(0, 0.5f, 0);
    auto scale = DirectX::XMMatrixScaling(DEFAULT_SIZE, length, DEFAULT_SIZE);
    DirectX::XMFLOAT4 _(0, 0, 0, 1);
    auto r = DirectX::XMMATRIX(X, Y, Z, DirectX::XMLoadFloat4(&_));

    auto shape = center * scale * r;
    DirectX::XMStoreFloat4x4(&node->ShapeMatrix, shape);
  }

  for (auto& child : node->Children) {
    CalcShape(child, scaling);
  }
}

void
Cuber::Update(const gltf::Scene& scene)
{
  Instances.clear();
  UpdateRecursive(scene.m_roots[0], DirectX::XMMatrixIdentity());
}

void
Cuber::UpdateRecursive(const std::shared_ptr<gltf::Node>& node,
                       DirectX::XMMATRIX parent)
{
  Instances.push_back({});
  auto m = node->Transform.ScalingTranslationMatrix(m_scaling) * parent;
  auto shape = DirectX::XMLoadFloat4x4(&node->ShapeMatrix);
  DirectX::XMStoreFloat4x4(&Instances.back(), shape * m);
  for (auto& child : node->Children) {
    UpdateRecursive(child, m);
  }
}

void
Cuber::Render(const ViewProjection& camera)
{
  m_cuber->Render(
    camera.projection, camera.view, Instances.data(), Instances.size());
  m_liner->Render(camera.projection, camera.view, m_lines);
}
