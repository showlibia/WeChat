//
// Created by matrix on 2/10/25.
//

#include "RPCConPool.h"

RPCConPool::RPCConPool(std::size_t pool_size, const std::string &host,
                       const std::string &port)
    : _b_stop(false) , _pool_size(pool_size), _host(host), _port(port) {
  if (pool_size == 0) {
    throw std::invalid_argument("pool_size must be greater than 0");
  }
  for (int i = 0; i < _pool_size; ++i) {
    std::shared_ptr<Channel> channel = grpc::CreateChannel(
        host + ":" + port, grpc::InsecureChannelCredentials());
    _connections.push(VerifyService::NewStub(channel));
  }
}

RPCConPool::~RPCConPool() {
  std::cout << __FUNCTION__ << std::endl;
  Close();
  std::unique_lock<std::mutex> lock(_mutex);
  while (!_connections.empty()) {
    _connections.pop();
  }
}

std::shared_ptr<VerifyService::Stub> RPCConPool::GetConnection() {
  std::unique_lock<std::mutex> lock(_mutex);
  _cv.wait(lock, [this]() {
    if (_b_stop) {
      return true;
    }
    return !_connections.empty();
  });
  if (_b_stop) {
    return nullptr;
  }
  auto connection = std::move(_connections.front());
  _connections.pop();

  // 自定义删除器，当shared_ptr的引用计数为0时，自动归还连接
  return std::shared_ptr<VerifyService::Stub>(
      connection.release(), [this](VerifyService::Stub *ptr) {
        // 当shared_ptr的引用计数为0时，自动归还连接
        std::unique_ptr<VerifyService::Stub> conn(ptr);
        {
          std::lock_guard<std::mutex> lock(_mutex);
          // 如果池未关闭，则归还连接；否则直接销毁连接
          if (!_b_stop) {
            _connections.push(std::move(conn));
          }
        }
        // 通知等待线程（既可能是
        // GetConnection，也可能是析构函数等待所有连接归还）
        _cv.notify_all();
      });
}

void RPCConPool::Close() {
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _b_stop = true;
  }
  _cv.notify_all();
}