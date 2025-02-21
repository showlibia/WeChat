#include "StatusServiceImpl.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"
#include "const.h"
#include <algorithm>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <climits>
#include <grpcpp/support/status.h>
#include <mutex>
#include <sstream>
#include <vector>

std::string generate_unique_id() {
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  return boost::uuids::to_string(uuid);
}

StatusServiceImpl::StatusServiceImpl() {
  auto &cfg = ConfigMgr::Instance();
  auto server_list = cfg["chatservers"]["name"];

  std::vector<std::string> words;
  std::stringstream ss(server_list);
  std::string word;

  while (std::getline(ss, word, ',')) {
    words.push_back(word);
  }

  for (auto &word : words) {
    if (cfg[word]["name"].empty()) {
      continue;
    }

    ChatServer server;
    server.host = cfg[word]["host"];
    server.port = cfg[word]["port"];
    server.name = cfg[word]["name"];
    _servers[server.name] = server;
  }
}

Status StatusServiceImpl::GetChatServer(ServerContext *context,
                                        const GetChatServerReq *request,
                                        GetChatServerRsp *response) {
  const auto &server = getChatServer();
  response->set_host(server.host);
  response->set_port(server.port);
  response->set_token(generate_unique_id());
  response->set_error(ErrorCodes::Success);
  insertToken(request->uid(), response->token());
  return Status::OK;
}

Status StatusServiceImpl::Login(ServerContext *context, const LoginReq *request,
                                LoginRsp *response) {
  auto uid = request->uid();
  auto token = request->token();

  std::string uid_str = std::to_string(uid);
  std::string token_key = USERTOKENPREFIX + uid_str;
  std::string token_value = "";
  bool success = RedisMgr::GetInstance()->Get(token_key, token_value);
  if (success) {
    response->set_error(ErrorCodes::Success);
    return Status::OK;
  }

  if (token_value != token) {
    response->set_error(ErrorCodes::TokenInvalid);
    return Status::OK;
  }

  response->set_error(ErrorCodes::Success);
  response->set_uid(uid);
  response->set_token(token);
  return Status::OK;
}

ChatServer StatusServiceImpl::getChatServer() {
  std::lock_guard<std::mutex> lock(_mutex);
  auto getConCount = [](const ChatServer &server) {
    auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server.name);
    return count_str.empty() ? INT_MAX : std::stoi(count_str);
  };

  auto minServerIt =
      std::min_element(_servers.begin(), _servers.end(),
                       [&getConCount](const auto &a, const auto &b) {
                         return getConCount(a.second) < getConCount(b.second);
                       });
  if (minServerIt != _servers.end()) {
    auto &minServer = minServerIt->second;
    minServer.con_count = getConCount(minServer);
    return minServer;
  }
  return _servers.begin()->second;
}

void StatusServiceImpl::insertToken(int uid, std::string token) {
  std::string uid_str = std::to_string(uid);
  std::string token_key = USERTOKENPREFIX + uid_str;
  RedisMgr::GetInstance()->Set(token_key, token);
}