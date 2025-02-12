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

using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::VerifyService;

/**
 * @brief RPCConPool 是一个 gRPC 连接池。
 *
 * 该类管理到 VerifyService 服务器的 gRPC 连接池。
 * 它允许高效地重用连接并确保线程安全。
 */

class RPCConPool {
public:
  RPCConPool(std::size_t pool_size, const std::string &host,
             const std::string &port);
  ~RPCConPool();
  std::shared_ptr<VerifyService::Stub> GetConnection();
  void Close();

private:
  std::atomic_bool _b_stop;
  std::size_t _pool_size;
  std::string _host;
  std::string _port;
  std::queue<std::unique_ptr<VerifyService::Stub>> _connections;
  std::mutex _mutex;
  std::condition_variable _cv;
};

#endif //SERVER_RPCCONPOOL_H
