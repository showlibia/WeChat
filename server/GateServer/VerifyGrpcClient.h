//
// Created by matrix on 2/8/25.
//

#ifndef SERVER_VERIFYGRPCCLIENT_H
#define SERVER_VERIFYGRPCCLIENT_H

#include <grpcpp/grpcpp.h>
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>
#include "const.h"
#include "Singleton.h"
#include "message.grpc.pb.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::VerifyService;
using message::GetVerifyReq;
using message::GetVerifyRsp;

/**
 * @brief RPCConPool 是一个 gRPC 连接池。
 *
 * 该类管理到 VerifyService 服务器的 gRPC 连接池。
 * 它允许高效地重用连接并确保线程安全。
 */

class RPCConPool {
public:
    RPCConPool(std::size_t pool_size, const std::string & host, const std::string & port);
    ~RPCConPool();
    std::shared_ptr<VerifyService::Stub> GetConnection();
    void Close();
private:
    std::atomic_bool _b_stop;
    std::size_t _pool_size;
    std::string _port;
    std::string _host;
    std::queue<std::unique_ptr<VerifyService::Stub>> _connections;
    std::mutex _mutex;
    std::condition_variable _cv;
};

/**
 * @brief VerifyGrpcClient 是一个单例类，用于向 VerifyService 服务器发送 gRPC 请求。
 *
 * 该类是一个单例类，它管理到 VerifyService 服务器的 gRPC 连接池。
 * 它提供了一个 GetVerifyCode 方法，用于向 VerifyService 服务器发送 GetVerifyCode 请求。
 */
class VerifyGrpcClient : public Singleton<VerifyGrpcClient>{
    friend class Singleton<VerifyGrpcClient>;
public:
    GetVerifyRsp GetVerifyCode(const std::string & email);
private:
    VerifyGrpcClient();
    std::unique_ptr<RPCConPool> _rpc_pool;
};


#endif //SERVER_VERIFYGRPCCLIENT_H
