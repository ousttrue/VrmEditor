#include "BvhPanel.h"
#include <asio.hpp>
#include <imgui.h>
#include <thread>
#include <vrm/bvhscene.h>
#include <vrm/scene.h>
#include <vrm/srht_sender.h>
#include <vrm/timeline.h>

class BvhPanelImpl
{
  asio::io_context m_io;
  asio::executor_work_guard<asio::io_context::executor_type> m_work;

  libvrm::UdpSender m_sender;
  std::shared_ptr<libvrm::bvh::Bvh> m_bvh;
  asio::ip::udp::endpoint m_ep;
  bool m_enablePackQuat = false;

  std::vector<DirectX::XMFLOAT4X4> m_instances;
  std::shared_ptr<libvrm::gltf::Scene> m_scene;
  std::shared_ptr<libvrm::IntervalTimer> m_clock;

public:
  BvhPanelImpl()
    : m_work(asio::make_work_guard(m_io))
    , m_sender(m_io)
    , m_ep(asio::ip::address::from_string("127.0.0.1"), 54345)
  {
    m_scene = std::make_shared<libvrm::gltf::Scene>();
  }

  ~BvhPanelImpl() { m_work.reset(); }

  void SetBvh(const std::shared_ptr<libvrm::bvh::Bvh>& bvh)
  {
    m_bvh = bvh;
    if (!m_bvh) {
      return;
    }

    libvrm::bvh::InitializeSceneFromBvh(m_scene, m_bvh);
    m_scene->m_roots[0]->UpdateShapeInstanceRecursive(
      DirectX::XMMatrixIdentity(), m_instances);

    m_sender.SendBvhSkeleton(m_ep, m_bvh);

    m_clock = std::make_shared<libvrm::IntervalTimer>(
      m_io,
      std::chrono::duration_cast<std::chrono::nanoseconds>(m_bvh->frame_time),
      [this](auto time) {
        auto index = m_bvh->TimeToIndex(time);
        auto frame = m_bvh->GetFrame(index);

        m_sender.SendBvhFrame(m_ep, m_bvh, frame, m_enablePackQuat);

        UpdateScene(frame);
      });
  }

  void UpdateGui()
  {
    m_io.poll();

    if (!m_bvh) {
      return;
    }
    // bvh panel
    ImGui::Begin("BVH");

    ImGui::LabelText("bvh", "%zu joints", m_bvh->joints.size());

    ImGui::Checkbox("use quaternion pack32", &m_enablePackQuat);

    if (ImGui::Button("send skeleton")) {
      m_sender.SendBvhSkeleton(m_ep, m_bvh);
    }

    ImGui::End();
  }

  void UpdateScene(const libvrm::bvh::Frame& frame)
  {
    libvrm::bvh::UpdateSceneFromBvhFrame(
      m_scene, m_scene->m_roots[0], m_bvh, frame, m_bvh->GuessScaling());
    m_scene->m_roots[0]->CalcWorldMatrix(true);
    m_instances.clear();
    m_scene->m_roots[0]->UpdateShapeInstanceRecursive(
      DirectX::XMMatrixIdentity(), m_instances);
  }

  std::span<const DirectX::XMFLOAT4X4> GetCubes() { return m_instances; }
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
