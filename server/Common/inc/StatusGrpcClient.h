#ifndef SERVER_STATUSGRPCLIENT_H
#define SERVER_STATUSGRPCLIENT_H

#include "RPCConPool.h"
#include "Singleton.h"
#include <memory>

class StatusGrpcClient : public Singleton<StatusGrpcClient> {
  friend class Singleton<StatusGrpcClient>;

public:
  ~StatusGrpcClient() {}
  GetChatServerRsp GetChatServer(int uid);
  LoginRsp Login(int uid, std::string token);
private:
  StatusGrpcClient();
  std::unique_ptr<RPCConPool<StatusService>> _pool;
};

#endif // SERVER_STATUSGRPCLIENT_H