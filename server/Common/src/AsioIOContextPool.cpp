//
// Created by matrix on 2/9/25.
//

#include "AsioIOContextPool.h"
#include "Logger.h"
#include <boost/log/trivial.hpp>

AsioIOContextPool::AsioIOContextPool(std::size_t size)
    : _ioContexts(size), _works(size), _nextIOContext(0)
{
    for(std::size_t i = 0; i < size; ++i)
    {
        // 绑定每个io_context到work, 防止io_context退出
        _works[i] = std::make_unique<Work>(_ioContexts[i].get_executor());
    }

    for(std::size_t i = 0; i < size; ++i)
    {
        _threads.emplace_back([this, i](){
            _ioContexts[i].run();
        });
    }
}

net::io_context &AsioIOContextPool::GetIOContext() {
    auto &ioContext = _ioContexts[_nextIOContext++];
    if(_nextIOContext == _ioContexts.size())
    {
        _nextIOContext = 0;
    }
    return ioContext;
}

AsioIOContextPool::~AsioIOContextPool() {
    Stop();
    LOG(info) << std::endl;
}

void AsioIOContextPool::Stop() {
    // 仅仅work.reset()是不够的，因为io_context可能在等待事件
    // 当iocontext已经绑定读写事件后，需要调用stop来停止io_context
    for (auto &work : _works) {
        work.reset();
    }

    for(auto &thread: _threads) {
        thread.join();
    }
}