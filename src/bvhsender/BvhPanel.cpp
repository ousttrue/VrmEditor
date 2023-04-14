#include "BvhPanel.h"
#include "Animation.h"
#include "UdpSender.h"
#include <asio.hpp>
#include <imgui.h>
#include <thread>
#include <vrm/bvhresolver.h>
#include <vrm/scene.h>

class BvhPanelImpl
{
  asio::io_context io_;
  asio::executor_work_guard<asio::io_context::executor_type> work_;
  Animation animation_;
  UdpSender sender_;
  std::thread thread_;
  std::shared_ptr<libvrm::bvh::Bvh> bvh_;
  asio::ip::udp::endpoint ep_;
  bool enablePackQuat_ = false;
  std::vector<int> parentMap_;

  std::vector<DirectX::XMFLOAT4X4> instancies_;
  std::mutex mutex_;
  std::shared_ptr<libvrm::gltf::Scene> m_scene;

public:
  BvhPanelImpl()
    : work_(asio::make_work_guard(io_))
    , animation_(io_)
    , sender_(io_)
    , ep_(asio::ip::address::from_string("127.0.0.1"), 54345)
  {
    animation_.OnFrame([self = this](const libvrm::bvh::Frame& frame) {
      self->sender_.SendFrame(
        self->ep_, self->bvh_, frame, self->enablePackQuat_);
    });
    thread_ = std::thread([self = this]() {
      try {
        self->io_.run();
        std::cout << "[asio] end" << std::endl;
      } catch (std::exception const& e) {
        std::cout << "[asio] catch" << e.what() << std::endl;
      }
    });

    // bind bvh animation to renderer
    animation_.OnFrame([self = this](const libvrm::bvh::Frame& frame) {
      self->SyncFrame(frame);
    });

    m_scene = std::make_shared<libvrm::gltf::Scene>();
  }

  ~BvhPanelImpl()
  {
    animation_.Stop();
    work_.reset();
    thread_.join();
  }

  void SetBvh(const std::shared_ptr<libvrm::bvh::Bvh>& bvh)
  {
    bvh_ = bvh;
    if (!bvh_) {
      return;
    }
    animation_.SetBvh(bvh);
    for (auto& joint : bvh_->joints) {
      parentMap_.push_back(joint.parent);
    }
    sender_.SendSkeleton(ep_, bvh_);

    libvrm::bvh::SetBvh(m_scene, bvh_);
    m_scene->m_roots[0]->UpdateShapeInstanceRecursive(
      DirectX::XMMatrixIdentity(), instancies_);
  }

  void UpdateGui()
  {
    if (!bvh_) {
      return;
    }
    // bvh panel
    ImGui::Begin("BVH");

    ImGui::LabelText("bvh", "%zu joints", bvh_->joints.size());

    ImGui::Checkbox("use quaternion pack32", &enablePackQuat_);

    if (ImGui::Button("send skeleton")) {
      sender_.SendSkeleton(ep_, bvh_);
    }

    ImGui::End();
  }

  void SyncFrame(const libvrm::bvh::Frame& frame)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    libvrm::bvh::ResolveFrame(
      m_scene, m_scene->m_roots[0], bvh_, frame, bvh_->GuessScaling());
    m_scene->m_roots[0]->CalcWorldMatrix(true);
    instancies_.clear();
    m_scene->m_roots[0]->UpdateShapeInstanceRecursive(
      DirectX::XMMatrixIdentity(), instancies_);
  }

  std::span<const DirectX::XMFLOAT4X4> GetCubes()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return instancies_;
  }
};

BvhPanel::BvhPanel()
  : impl_(new BvhPanelImpl)
{
}
BvhPanel::~BvhPanel()
{
  delete impl_;
}
void
BvhPanel::SetBvh(const std::shared_ptr<libvrm::bvh::Bvh>& bvh)
{
  impl_->SetBvh(bvh);
}
void
BvhPanel::UpdateGui()
{
  impl_->UpdateGui();
}
std::span<const DirectX::XMFLOAT4X4>
BvhPanel::GetCubes()
{
  return impl_->GetCubes();
}
