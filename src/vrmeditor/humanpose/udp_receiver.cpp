#include "udp_receiver.h"
#include <iostream>
#include <unordered_map>

#ifdef _WIN32
#define _WIN32_WINNT 0x0601
#endif
#include <asio.hpp>

struct UdpHandle
{
  asio::ip::udp::socket m_sock;
  asio::ip::udp::endpoint m_sender_endpoint;
  uint8_t m_data[8192];

  UdpHandle(asio::io_context& io, uint16_t port)
    : m_sock(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port))
  {
  }

  void Start(const OnRecv& onRecv)
  {
    m_sock.async_receive_from(
      asio::buffer(m_data, sizeof(m_data)),
      m_sender_endpoint,
      [onRecv, this](std::error_code ec, std::size_t bytes_recvd) {
        if (!ec && bytes_recvd > 0) {
          onRecv(std::span(m_data, bytes_recvd));
          Start(onRecv);
        } else {
          // error
        }
      });
  }
};

struct UdpReceiverImpl
{
  asio::io_context m_io;
  asio::executor_work_guard<asio::io_context::executor_type> m_work;
  std::unordered_map<uint16_t, std::shared_ptr<UdpHandle>> m_handles;

  UdpReceiverImpl()
    : m_work(asio::make_work_guard(m_io))
  {
  }

  void Update() { m_io.poll(); }

  void Start(uint16_t port, const OnRecv& callback)
  {
    try {
      auto receiver = std::make_shared<UdpHandle>(m_io, port);
      m_handles.insert({ port, receiver });
      receiver->Start(callback);
    } catch (const std::system_error& ex) {
      std::cerr << ex.what() << std::endl;
    } catch (const std::runtime_error& ex) {
      std::cerr << ex.what() << std::endl;
    }
  }

  void Stop(uint16_t port)
  {
    auto it = m_handles.find(port);
    if (it != m_handles.end()) {
      m_handles.erase(it);
    }
  }
};

UdpReceiver::UdpReceiver()
  : m_impl(new UdpReceiverImpl)
{
}

UdpReceiver::~UdpReceiver()
{
  delete m_impl;
}

void
UdpReceiver::Update()
{
  m_impl->Update();
}

void
UdpReceiver::Start(uint16_t port, const OnRecv& callback)
{
  m_impl->Start(port, callback);
}

void
UdpReceiver::Stop(uint16_t port)
{
  m_impl->Stop(port);
}
