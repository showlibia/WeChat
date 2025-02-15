#ifndef SERVER_STATUS_SERVICE_IMPL_H
#define SERVER_STATUS_SERVICE_IMPL_H

#include "message.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <vector>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;

struct ChatServer {
  std::string host;
  std::string port;
};

class StatusServiceImpl final : public StatusService::Service {
public:
  StatusServiceImpl();
  Status GetChatServer(ServerContext *context, const GetChatServerReq *request,
                       GetChatServerRsp *response) override;
private:
  std::vector<ChatServer> _servers;
  int _server_index;
  };

#endif