#ifndef SERVER_CHATGRPCCLIENT_H
#define SERVER_CHATGRPCCLIENT_H

#include "ConfigMgr.h"
#include "Singleton.h"
#include "const.h"
#include "data.h"
#include "RPCConPool.h"
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <grpcpp/grpcpp.h>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <queue>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::AddFriendReq;
using message::AddFriendRsp;

using message::AuthFriendReq;
using message::AuthFriendRsp;

using message::ChatService;
using message::GetChatServerRsp;
using message::LoginReq;
using message::LoginRsp;

using message::TextChatData;
using message::TextChatMsgReq;
using message::TextChatMsgRsp;

class ChatGrpcClient : public Singleton<ChatGrpcClient> {
  friend class Singleton<ChatGrpcClient>;

public:
  ~ChatGrpcClient();
  AddFriendRsp NotifyAddFriend(std::string ip, const AddFriendReq &req);
  AuthFriendRsp NotifyAuthFriend(std::string server_ip,
                                 const AuthFriendReq &req);
  bool GetBaseInfo(std::string base_key, int uid,
                   std::shared_ptr<UserInfo> &userinfo);
  TextChatMsgRsp NotifyTextChatMsg(std::string server_ip,
                                   const TextChatMsgReq &req,
                                   const Json::Value &rtvalue);

private:
  ChatGrpcClient();
  std::unordered_map<std::string, std::unique_ptr<RPCConPool<ChatService>>> _pools;
};

#endif // SERVER_CHATGRPCCLIENT_H