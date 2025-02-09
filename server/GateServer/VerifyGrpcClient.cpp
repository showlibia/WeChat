//
// Created by matrix on 2/8/25.
//

#include "VerifyGrpcClient.h"
#include "ConfigMgr.h"


GetVerifyRsp VerifyGrpcClient::GetVerifyCode(const std::string & email) {
    ClientContext context;
    GetVerifyReq req;
    GetVerifyRsp reply;
    req.set_email(email);

    auto stub = _rpc_pool->GetConnection();
    Status status = stub->GetVerifyCode(&context, req, &reply);
    if (!status.ok()) {
        reply.set_error(ErrorCodes::RPCFailed);
    }
    return reply;
}

VerifyGrpcClient::VerifyGrpcClient() {
    auto &gCfgMgr = ConfigMgr::Instance();
    std::string host = gCfgMgr["VerifyServer"]["host"];
    std::string port = gCfgMgr["VerifyServer"]["port"];
    _rpc_pool.reset(new RPCConPool(5, host, port));
}

RPCConPool::RPCConPool(std::size_t pool_size, const std::string &host, const std::string &port)
   : _pool_size(pool_size), _host(host), _port(port), _b_stop(false)
    {
        for (int i = 0; i < _pool_size; ++i) {
            std::shared_ptr<Channel> channel = grpc::CreateChannel(host+":"+port, grpc::InsecureChannelCredentials());
            _connections.push(VerifyService::NewStub(channel));
        }
    }

RPCConPool::~RPCConPool() {
    std::lock_guard<std::mutex> lock(_mutex);
    Close();
    while(!_connections.empty()) {
        _connections.pop();
    }
}

std::shared_ptr<VerifyService::Stub> RPCConPool::GetConnection() {
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock,[this](){
        if(_b_stop) {
            return true;
        }
        return !_connections.empty();
    });
    if(_b_stop) {
        return nullptr;
    }
    auto connection = std::move(_connections.front());
    _connections.pop();

    // 自定义删除器，当shared_ptr的引用计数为0时，自动归还连接
    return std::shared_ptr<VerifyService::Stub>(
            connection.release(),
            [this](VerifyService::Stub* ptr){
                // 当shared_ptr的引用计数为0时，自动归还连接
                std::unique_lock<std::mutex> lock(_mutex);
                _connections.push(std::unique_ptr<VerifyService::Stub>(ptr));
                lock.unlock();
                _cv.notify_one();
            });
}

void RPCConPool::Close() {
    _b_stop = true;
    _cv.notify_all();
}