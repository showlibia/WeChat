#include "ChatGrpcClient.h"
#include <boost/algorithm/string.hpp>
#include <memory>
#include <type_traits>
#include <vector>

ChatGrpcClient::ChatGrpcClient() {
  auto &cfg = ConfigMgr::Instance();
  cfg.LoadConfig("self.ini");

  auto servers = cfg["PeerServer"]["servers"];
  std::vector<std::string> words;

  boost::split(words, servers, boost::is_any_of(","));
  for (auto &word : words) {
    if (cfg[word]["name"].empty()) {
      continue;
    }

    _pools[cfg[word]["name"]] = std::make_unique<RPCConPool<ChatService>>(
        5, cfg[word]["host"], cfg[word]["port"]);
  }
}

AddFriendRsp ChatGrpcClient::NotifyAddFriend(std::string server_ip,
                                             const AddFriendReq &req) {
  AddFriendRsp rsp;
  return rsp;
}

AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(std::string server_ip,
                                               const AuthFriendReq &req) {
  AuthFriendRsp rsp;
  return rsp;
}

bool ChatGrpcClient::GetBaseInfo(std::string base_key, int uid,
                                 std::shared_ptr<UserInfo> &userinfo) {
  return true;
}

TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(std::string server_ip,
                                                 const TextChatMsgReq &req,
                                                 const Json::Value &rtvalue) {

  TextChatMsgRsp rsp;
  return rsp;
}