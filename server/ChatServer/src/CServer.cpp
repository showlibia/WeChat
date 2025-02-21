#include "CServer.h"
#include "AsioIOContextPool.h"
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>

CServer::CServer(boost::asio::io_context &ioc, short port)
    : _ioc(ioc), _port(port), _acceptor(ioc, tcp::endpoint(tcp::v4(), port)) {
  StartAccept();
}

CServer::~CServer() { _acceptor.close(); }

void CServer ::HandleAccept(std::shared_ptr<CSession> session,
                            const boost::system::error_code &ec) {
  if (!ec) {
    session->Start();
    std::lock_guard<std::mutex> lock(_mutex);
    _sessions.insert(std::make_pair(session->GetUuid(), session));
  } else {
    std::cerr << "Error: " << ec.what() << std::endl;
  }
  StartAccept();
}

void CServer::StartAccept() {
  auto &io_context = AsioIOContextPool::GetInstance()->GetIOContext();
  std::shared_ptr<CSession> session = std::make_shared<CSession>(io_context, this);
  _acceptor.async_accept(session->GetSocket(),
                         std::bind(&CServer::HandleAccept, this, session,
                                   std::placeholders::_1));
}

void CServer::ClearSession(std::string uuid) {
    std::lock_guard<std::mutex> lock(_mutex);
    _sessions.erase(uuid);
}