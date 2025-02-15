//
// Created by matrix on 2/10/25.
//

#ifndef SERVER_RPCCONPOOL_H
#define SERVER_RPCCONPOOL_H

#include "message.grpc.pb.h"
#include <atomic>
#include <condition_variable>
#include <grpcpp/grpcpp.h>
#include <mutex>
#include <queue>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::StatusService;
using message::VerifyService;

/**
 * @brief Template class for managing a pool of RPC service connections
 * 
 * @tparam ServiceType The RPC service type to create stubs for
 *
 * This class implements a thread-safe connection pool for gRPC service stubs.
 * It maintains a fixed number of connections that can be borrowed and returned
 * to the pool.
 *
 * @note ServiceType must provide a nested Stub type
 */
template <typename ServiceType> class RPCConPool {
  using ServiceStubType = typename ServiceType::Stub;

public:
  RPCConPool(std::size_t pool_size, const std::string &host,
             const std::string &port);
  ~RPCConPool();
  std::shared_ptr<ServiceStubType> GetConnection();
  void Close();

private:
  std::atomic_bool _b_stop;
  std::size_t _pool_size;
  std::string _host;
  std::string _port;
  std::queue<std::unique_ptr<ServiceStubType>> _connections;
  std::mutex _mutex;
  std::condition_variable _cv;
};

template <typename ServiceType>
RPCConPool<ServiceType>::RPCConPool(std::size_t pool_size,
                                    const std::string &host,
                                    const std::string &port)
    : _b_stop(false), _pool_size(pool_size), _host(host), _port(port) {
  if (pool_size == 0) {
    throw std::invalid_argument("pool_size must be greater than 0");
  }
  for (int i = 0; i < _pool_size; ++i) {
    std::shared_ptr<Channel> channel = grpc::CreateChannel(
        host + ":" + port, grpc::InsecureChannelCredentials());
    _connections.push(std::make_unique<ServiceStubType>(channel));
  }
}

template <typename ServiceType> RPCConPool<ServiceType>::~RPCConPool() {
  std::cout << __FUNCTION__ << std::endl;
  Close();
  std::unique_lock<std::mutex> lock(_mutex);
  while (!_connections.empty()) {
    _connections.pop();
  }
}

template <typename ServiceType>
std::shared_ptr<typename RPCConPool<ServiceType>::ServiceStubType>
RPCConPool<ServiceType>::GetConnection() {
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
  return std::shared_ptr<ServiceStubType>(
      connection.release(), [this](ServiceStubType *ptr) {
        // 当shared_ptr的引用计数为0时，自动归还连接
        std::unique_ptr<ServiceStubType> conn(ptr);
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

template <typename ServiceType> void RPCConPool<ServiceType>::Close() {
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _b_stop = true;
  }
  _cv.notify_all();
}

#endif // SERVER_RPCCONPOOL_H
