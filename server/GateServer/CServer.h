//
// Created by matrix on 2/6/25.
//

#ifndef SERVER_CSERVER_H
#define SERVER_CSERVER_H

#include "const.h"

class CServer : public std::enable_shared_from_this<CServer>{
public:
    CServer(net::io_context& ioc, unsigned short& port);
    void Start();
private:
    // 接收对端的连接，因此需要和底层的事件循环进行绑定，借助io_context进行绑定
    tcp::acceptor _acceptor;
    net::io_context& _ioc;
    tcp::socket _socket;
};


#endif //SERVER_CSERVER_H
