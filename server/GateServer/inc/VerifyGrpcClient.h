//
// Created by matrix on 2/8/25.
//

#ifndef SERVER_VERIFYGRPCCLIENT_H
#define SERVER_VERIFYGRPCCLIENT_H

#include "Singleton.h"
#include "RPCConPool.h"

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
