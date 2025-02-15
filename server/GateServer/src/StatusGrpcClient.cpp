#include "StatusGrpcClient.h"
#include "ConfigMgr.h"

GetChatServerRsp StatusGrpcClient::GetChatServer(int uid) {
  ClientContext context;
  GetChatServerReq req;
  GetChatServerRsp reply;
  req.set_uid(uid);

  auto stub = _pool->GetConnection();
  Status status = stub->GetChatServer(&context, req, &reply);
  if (!status.ok()) {
    reply.set_error(ErrorCodes::RPCFailed);
  }
  return reply;
}

StatusGrpcClient::StatusGrpcClient() {
  auto &gCfgMgr = ConfigMgr::Instance();
  std::string host = gCfgMgr["StatusServer"]["host"];
  std::string port = gCfgMgr["StatusServer"]["port"];
  _pool.reset(new RPCConPool<StatusService>(5, host, port));
}