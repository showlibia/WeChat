//
// Created by matrix on 2/6/25.
//

#include "CServer.h"
#include "HttpConnection.h"

CServer::CServer(net::io_context& ioc, unsigned short &port)
    : _ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port)), _socket(ioc)
{}

void CServer::Start() {
    auto self = shared_from_this();
    _acceptor.async_accept(_socket, [self](beast::error_code ec) {
        try {
            // 出错则放弃此连接，并继续监听新连接
            if(ec) {
                self->Start();
                return ;
            }
            // 处理新连接，创建HttpConnection类管理新连接
            std::make_shared<HttpConnection>(std::move(self->_socket))->Start();
            self->Start();
        } catch (std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
            self->Start();
        }
    });
}