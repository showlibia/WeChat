#ifndef SERVER_STATUS_SERVICE_IMPL_H
#define SERVER_STATUS_SERVICE_IMPL_H

#include "message.grpc.pb.h"
#include "message.pb.h"
#include <grpcpp/grpcpp.h>
#include <grpcpp/server_context.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::LoginReq;
using message::LoginRsp;
using message::StatusService;

class ChatServer {
public:
  ChatServer() : host(""), port(""), name(""), con_count(0) {}
  ChatServer(const ChatServer &cs)
      : host(cs.host), port(cs.port), name(cs.name), con_count(cs.con_count) {}
  ChatServer &operator=(const ChatServer &cs) {
    if (&cs == this) {
      return *this;
    }

    host = cs.host;
    name = cs.name;
    port = cs.port;
    con_count = cs.con_count;
    return *this;
  }
  std::string host;
  std::string port;
  std::string name;
  int con_count;
};

class StatusServiceImpl final : public StatusService::Service {
public:
  StatusServiceImpl();
  Status GetChatServer(ServerContext *context, const GetChatServerReq *request,
                       GetChatServerRsp *response) override;
  ChatServer getChatServer();
  Status Login(ServerContext *, const LoginReq *, LoginRsp *) override;

private:
  void insertToken(int uid, std::string token);
  std::unordered_map<std::string, ChatServer> _servers;
  std::mutex _mutex;
};

#endif