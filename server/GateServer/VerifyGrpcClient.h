//
// Created by matrix on 2/8/25.
//

#ifndef SERVER_VERIFYGRPCCLIENT_H
#define SERVER_VERIFYGRPCCLIENT_H

#include <grpcpp/grpcpp.h>
#include "const.h"
#include "Singleton.h"
#include "message.grpc.pb.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::VerifyService;
using message::GetVerifyReq;
using message::GetVerifyRsp;

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>{
    friend class Singleton<VerifyGrpcClient>;
public:
    GetVerifyRsp GetVerifyCode(const std::string & email) {
        ClientContext context;
        GetVerifyReq req;
        GetVerifyRsp reply;
        req.set_email(email);

        Status status = _stub->GetVerifyCode(&context, req, &reply);
        if (!status.ok()) {
            reply.set_error(ErrorCodes::RPCFailed);
        }
        return reply;
    }
private:
    VerifyGrpcClient() {
        std::shared_ptr<Channel> channel = grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials());
        _stub = VerifyService::NewStub(channel);
    }
    std::unique_ptr<VerifyService::Stub> _stub;
};


#endif //SERVER_VERIFYGRPCCLIENT_H
