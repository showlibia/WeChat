//
// Created by matrix on 2/6/25.
//

#include "CServer.h"
#include "Logger.h"
#include "HttpConnection.h"
#include "AsioIOContextPool.h"

CServer::CServer(net::io_context& ioc, unsigned short &port)
    : _ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{}

void CServer::Start() {
    auto self = shared_from_this();
    auto& io_context = AsioIOContextPool::GetInstance()->GetIOContext();
    std::shared_ptr<HttpConnection> connection = std::make_shared<HttpConnection>(io_context);
    _acceptor.async_accept(connection->GetSocket(), [self, connection](beast::error_code ec) {
        try {
            // 出错则放弃此连接，并继续监听新连接
            if(ec) {
                self->Start();
                return ;
            }
            // 处理新连接，创建HttpConnection类管理新连接
            connection->Start();
            self->Start();
        } catch (std::exception& e) {
          LOG(warning) << "Error: " << e.what() << std::endl;
          self->Start();
        }
    });
}