//
// Created by matrix on 2/8/25.
//

#include "VerifyGrpcClient.h"
#include "ConfigMgr.h"

GetVerifyRsp VerifyGrpcClient::GetVerifyCode(const std::string &email) {
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