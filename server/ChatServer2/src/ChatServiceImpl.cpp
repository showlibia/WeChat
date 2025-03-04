#include "ChatServiceImpl.h"
#include "CSession.h"
#include "Logger.h" // 引入Logger类
#include "MysqlMgr.h"
#include "RedisMgr.h"
#include "UserMgr.h"
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>

ChatServiceImpl::ChatServiceImpl() {}

Status ChatServiceImpl::NotifyAddFriend(ServerContext *context,
                                        const AddFriendReq *request,
                                        AddFriendRsp *reply) {
  // 查找用户是否在本服务器
  auto touid = request->touid();
  auto session = UserMgr::GetInstance()->GetSession(touid);

  Defer defer([request, reply]() {
    reply->set_error(ErrorCodes::Success);
    reply->set_applyuid(request->applyuid());
    reply->set_touid(request->touid());
  });

  // 用户不在内存中则直接返回
  if (session == nullptr) {
    LOG(warning) << "User not in memory, touid: " << touid;
    return Status::OK;
  }

  // 在内存中则直接发送通知对方
  LOG(info) << "User login, uid: " << request->applyuid()
            << ", applyname: " << request->name()
            << ", nick: " << request->nick() << ", touid: " << request->touid();

  Json::Value rtvalue;
  rtvalue["error"] = ErrorCodes::Success;
  rtvalue["applyuid"] = request->applyuid();
  rtvalue["name"] = request->name();
  rtvalue["desc"] = request->desc();
  rtvalue["icon"] = request->icon();
  rtvalue["sex"] = request->sex();
  rtvalue["nick"] = request->nick();

  std::string return_str = rtvalue.toStyledString();

  session->Send(return_str, ID_NOTIFY_ADD_FRIEND_REQ);

  return Status::OK;
}

Status ChatServiceImpl::NotifyAuthFriend(ServerContext *context,
                                         const AuthFriendReq *request,
                                         AuthFriendRsp *response) {
  return Status::OK;
}

Status ChatServiceImpl::NotifyTextChatMsg(::grpc::ServerContext *context,
                                          const TextChatMsgReq *request,
                                          TextChatMsgRsp *response) {
  return Status::OK;
}

bool ChatServiceImpl::GetBaseInfo(std::string base_key, int uid,
                                  std::shared_ptr<UserInfo> &userinfo) {
  return true;
}
