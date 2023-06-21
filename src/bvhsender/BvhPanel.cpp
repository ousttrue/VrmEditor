#include "BvhPanel.h"
#include <imgui.h>
#include <thread>
#include <vrm/bvh/bvhscene.h>
#include <vrm/gltfroot.h>
#include <vrm/network/srht_sender.h>
#include <vrm/runtime_scene.h>
#include <vrm/timeline.h>

#ifdef _WIN32
#define _WIN32_WINNT 0x0601
#endif
#include <asio.hpp>

class BvhPanelImpl
{
  asio::io_context m_io;
  asio::executor_work_guard<asio::io_context::executor_type> m_work;

  libvrm::srht::UdpSender m_sender;
  std::shared_ptr<libvrm::bvh::Bvh> m_bvh;
  asio::ip::udp::endpoint m_ep;
  bool m_enablePackQuat = false;

  std::vector<cuber::Instance> m_instances;
  std::shared_ptr<libvrm::RuntimeScene> m_scene;
  std::shared_ptr<libvrm::IntervalTimer> m_clock;

public:
  BvhPanelImpl()
    : m_work(asio::make_work_guard(m_io))
    , m_sender(m_io)
    , m_ep(asio::ip::address::from_string("127.0.0.1"), 54345)
  {
    auto scene = std::make_shared<libvrm::GltfRoot>();
    m_scene = std::make_shared<libvrm::RuntimeScene>(scene);
  }

  ~BvhPanelImpl() { m_work.reset(); }

  void PushInstance(const libvrm::Instance& instance)
  {
    m_instances.push_back(*((const cuber::Instance*)&instance));
  }

  void SetBvh(const std::shared_ptr<libvrm::bvh::Bvh>& bvh)
  {
    m_bvh = bvh;
    if (!m_bvh) {
      return;
    }

    libvrm::bvh::InitializeSceneFromBvh(m_scene->m_base, m_bvh);
    m_scene->Reset();
    m_scene->m_roots[0]->UpdateShapeInstanceRecursive(
      DirectX::XMMatrixIdentity(),
      std::bind(&BvhPanelImpl::PushInstance, this, std::placeholders::_1));

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
      DirectX::XMMatrixIdentity(),
      std::bind(&BvhPanelImpl::PushInstance, this, std::placeholders::_1));
  }

  std::span<const cuber::Instance> GetCubes() { return m_instances; }
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
std::span<const cuber::Instance>
BvhPanel::GetCubes()
{
  return impl_->GetCubes();
}
