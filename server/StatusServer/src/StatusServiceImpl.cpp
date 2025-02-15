#include "StatusServiceImpl.h"
#include "ConfigMgr.h"
#include "const.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

std::string generate_unique_id() {
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  return boost::uuids::to_string(uuid);
}

StatusServiceImpl::StatusServiceImpl() : _server_index(0) {
  auto &cfg = ConfigMgr::Instance();
  ChatServer server;
  server.host = cfg["ChatServer1"]["host"];
  server.port = cfg["ChatServer1"]["port"];
  _servers.push_back(server);

  server.host = cfg["ChatServer2"]["host"];
  server.port = cfg["ChatServer2"]["port"];
  _servers.push_back(server);
}

Status StatusServiceImpl::GetChatServer(ServerContext *context,
                                        const GetChatServerReq *request,
                                        GetChatServerRsp *response) {
  _server_index = (_server_index++) % _servers.size();
  auto server = _servers[_server_index];
  response->set_host(server.host);
  response->set_port(server.port);
  response->set_token(generate_unique_id());
  response->set_error(ErrorCodes::Success);
  return Status::OK;
}