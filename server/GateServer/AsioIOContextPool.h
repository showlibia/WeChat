//
// Created by matrix on 2/9/25.
//

#ifndef SERVER_ASIOIOCONTEXTPOOL_H
#define SERVER_ASIOIOCONTEXTPOOL_H

#include "Singleton.h"
#include "const.h"
#include <vector>


class AsioIOContextPool : public Singleton<AsioIOContextPool>{
    friend class Singleton<AsioIOContextPool>;
public:
    using IOContext = net::io_context;
    using Work = net::io_context::work;
    using WorkPtr = std::unique_ptr<Work>;
    ~AsioIOContextPool();
    AsioIOContextPool(const AsioIOContextPool&) = delete;
    AsioIOContextPool& operator=(const AsioIOContextPool&) = delete;
    net::io_context & GetIOContext();
    void Stop();
private:
    explicit AsioIOContextPool(std::size_t size = std::thread::hardware_concurrency());
    std::vector<IOContext> _ioContexts;
    // work 绑定到io_context上，防止io_context退出
    std::vector<WorkPtr> _works;
    std::vector<std::thread> _threads;
    std::size_t _nextIOContext;
};


#endif //SERVER_ASIOIOCONTEXTPOOL_H
