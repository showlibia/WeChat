#include "StatusServiceImpl.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"
#include "const.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>
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

  // 使用 Boost 分割字符串
  std::vector<std::string> words;
  boost::split(words, server_list, boost::is_any_of(",")); // 按逗号分割

  for (auto &word : words) {
    boost::trim(word); // 去除字符串两端的空格

    if (word.empty() || cfg[word]["name"].empty()) {
      continue;
    }

    std::cout << __FUNCTION__ << " server name: " << word << std::endl;
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
  std::lock_guard<std::mutex> guard(_mutex);
  auto minServer = _servers.begin()->second;
  auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, minServer.name);
  if (count_str.empty()) {
    // 不存在则默认设置为最大
    minServer.con_count = INT_MAX;
  } else {
    minServer.con_count = std::stoi(count_str);
  }

  // 使用范围基于for循环
  for (auto &server : _servers) {

    if (server.second.name == minServer.name) {
      continue;
    }

    auto count_str =
        RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server.second.name);
    std::cout << __FUNCTION__ << "server name : " << server.second.name
              << " count : " << count_str << std::endl;
    if (count_str.empty()) {
      server.second.con_count = INT_MAX;
    } else {
      server.second.con_count = std::stoi(count_str);
    }

    if (server.second.con_count < minServer.con_count) {
      minServer = server.second;
    }
  }

  return minServer;
}

void StatusServiceImpl::insertToken(int uid, std::string token) {
  std::string uid_str = std::to_string(uid);
  std::string token_key = USERTOKENPREFIX + uid_str;
  RedisMgr::GetInstance()->Set(token_key, token);
}