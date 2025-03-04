#include "LogicSystem.h"
#include "ChatGrpcClient.h"
#include "ConfigMgr.h"
#include "Logger.h"
#include "MysqlMgr.h"
#include "MysqlPool.h"
#include "RedisMgr.h"
#include "StatusGrpcClient.h"
#include "UserMgr.h"
#include "const.h"
#include "message.pb.h"
#include "reader.h"
#include "value.h"
#include <json/json.h>
#include <memory>
#include <mutex>

LogicSystem::LogicSystem() : _b_stop(false) {
  RegisterCallBacks();
  _work = std::thread(&LogicSystem::DealMsg, this);
}

LogicSystem::~LogicSystem() {
  _b_stop = true;
  _consume.notify_all();
  _work.join();
}

void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg) {
  LOG(info) << std::endl;
  std::unique_lock<std::mutex> lock(_mutex);
  _msg_que.push(msg);
  if (_msg_que.size() == 1) {
    lock.unlock();
    _consume.notify_one();
  }
}

void LogicSystem::DealMsg() {
  for (;;) {
    std::unique_lock<std::mutex> lock(_mutex);
    while (_msg_que.empty() && !_b_stop) {
      _consume.wait(lock);
    }

    if (_b_stop) {
      while (!_msg_que.empty()) {
        auto msg_node = _msg_que.front();
        LOG(info) << "msg_id: " << msg_node->_recv_node->_msg_id << std::endl;
        auto call_back_iter = _fun_maps.find(msg_node->_recv_node->_msg_id);
        if (call_back_iter == _fun_maps.end()) {
          _msg_que.pop();
          continue;
        }
        call_back_iter->second(msg_node->_session,
                               msg_node->_recv_node->_msg_id,
                               std::string(msg_node->_recv_node->_data,
                                           msg_node->_recv_node->_cur_len));
        _msg_que.pop();
      }
      break;
    }

    // 如果没有停服，则说明队列中有数据
    auto msg_node = _msg_que.front();
    LOG(info) << "recv_msg id  is " << msg_node->_recv_node->_msg_id
              << std::endl;
    auto call_back_iter = _fun_maps.find(msg_node->_recv_node->_msg_id);
    if (call_back_iter == _fun_maps.end()) {
      _msg_que.pop();
      LOG(info) << "msg id [" << msg_node->_recv_node->_msg_id
                << "] handler not found" << std::endl;
      continue;
    }
    call_back_iter->second(msg_node->_session, msg_node->_recv_node->_msg_id,
                           std::string(msg_node->_recv_node->_data,
                                       msg_node->_recv_node->_cur_len));
    _msg_que.pop();
  }
}

void LogicSystem::RegisterCallBacks() {
  _fun_maps[MSG_CHAT_LOGIN] =
      std::bind(&LogicSystem::LoginHandler, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3);
  _fun_maps[ID_SEARCH_USER_REQ] =
      std::bind(&LogicSystem::SearchInfo, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3);
  _fun_maps[ID_ADD_FRIEND_REQ] =
      std::bind(&LogicSystem::AddFriendApply, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3);
}

bool LogicSystem::GetBaseInfo(std::string base_key, int uid,
                              std::shared_ptr<UserInfo> &userinfo) {
  std::string info = "";
  bool b_base = RedisMgr::GetInstance()->Get(base_key, info);
  if (b_base) {
    Json::Reader reader;
    Json::Value root;
    reader.parse(info, root);
    userinfo->uid = root["uid"].asInt();
    userinfo->name = root["name"].asString();
    userinfo->pwd = root["pwd"].asString();
    userinfo->email = root["email"].asString();
    userinfo->nick = root["nick"].asString();
    userinfo->desc = root["desc"].asString();
    userinfo->sex = root["sex"].asInt();
    userinfo->icon = root["icon"].asString();
    LOG(info) << "user login uid is  " << userinfo->uid << " name  is "
              << userinfo->name << " pwd is " << userinfo->pwd << " email is "
              << userinfo->email << std::endl;
  } else {
    // redis查询失败，从mysql查询
    std::shared_ptr<UserInfo> user_info = MysqlMgr::GetInstance()->GetUser(uid);
    if (user_info == nullptr) {
      return false;
    }

    userinfo = user_info;

    // 将mysql查询结果存入redis
    Json::Value redis_root;
    redis_root["uid"] = uid;
    redis_root["pwd"] = userinfo->pwd;
    redis_root["name"] = userinfo->name;
    redis_root["email"] = userinfo->email;
    redis_root["nick"] = userinfo->nick;
    redis_root["desc"] = userinfo->desc;
    redis_root["sex"] = userinfo->sex;
    redis_root["icon"] = userinfo->icon;
    RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());
  }

  return true;
}

void LogicSystem::AddFriendApply(std::shared_ptr<CSession> session,
                                 const short &msg_id,
                                 const std::string &msg_data) {
  Json::Reader reader;
  Json::Value root;
  reader.parse(msg_data, root);
  auto uid = root["uid"].asInt();
  auto applyname = root["applyname"].asString();
  auto bakname = root["bakname"].asString();
  auto touid = root["touid"].asInt();
  LOG(info) << "user login uid is  " << uid << " applyname  is " << applyname
            << " bakname is " << bakname << " touid is " << touid << std::endl;

  Json::Value rtvalue;
  rtvalue["error"] = ErrorCodes::Success;
  Defer defer([&rtvalue, session]() {
    std::string return_str = rtvalue.toStyledString();
    session->Send(return_str, ID_ADD_FRIEND_RSP);
  });

  // 先更新数据库
  MysqlMgr::GetInstance()->AddFriendApply(uid, touid);

  // 查询redis 查找touid对应的server ip
  auto to_str = std::to_string(touid);
  auto to_ip_key = USERIPPREFIX + to_str;
  std::string to_ip_value = "";
  bool b_ip = RedisMgr::GetInstance()->Get(to_ip_key, to_ip_value);
  if (!b_ip) {
    return;
  }

  auto &cfg = ConfigMgr::Instance();
  auto self_name = cfg["SelfServer"]["name"];

  std::string base_key = USER_BASE_INFO + std::to_string(uid);
  auto apply_info = std::make_shared<UserInfo>();
  bool b_info = GetBaseInfo(base_key, uid, apply_info);

  // 直接通知对方有申请消息
  if (to_ip_value == self_name) {
    auto session = UserMgr::GetInstance()->GetSession(touid);
    if (session) {
      // 在内存中则直接发送通知对方
      LOG(info) << ": user login uid is  " << uid << " applyname  is "
                << applyname << " bakname is " << bakname << " touid is "
                << touid << std::endl;
      Json::Value notify;
      notify["error"] = ErrorCodes::Success;
      notify["applyuid"] = uid;
      notify["name"] = applyname;
      notify["desc"] = "";
      if (b_info) {
        notify["icon"] = apply_info->icon;
        notify["sex"] = apply_info->sex;
        notify["nick"] = apply_info->nick;
      }
      std::string return_str = notify.toStyledString();
      session->Send(return_str, ID_NOTIFY_ADD_FRIEND_REQ);
    }

    return;
  }

  message::AddFriendReq add_req;
  add_req.set_applyuid(uid);
  add_req.set_touid(touid);
  add_req.set_name(applyname);
  add_req.set_desc("");
  if (b_info) {
    add_req.set_icon(apply_info->icon);
    add_req.set_sex(apply_info->sex);
    add_req.set_nick(apply_info->nick);
  }

  // 发送通知
  ChatGrpcClient::GetInstance()->NotifyAddFriend(to_ip_value, add_req);
}

void LogicSystem::SearchInfo(std::shared_ptr<CSession> session,
                             const short &msg_id, const std::string &msg_data) {
  Json::Reader reader;
  Json::Value root;
  reader.parse(msg_data, root);
  auto uid_str = root["uid"].asString();
  LOG(info) << "user SearchInfo uid is  " << uid_str << std::endl;

  Json::Value rtvalue;

  Defer deder([&rtvalue, session]() {
    std::string return_str = rtvalue.toStyledString();
    session->Send(return_str, ID_SEARCH_USER_RSP);
  });

  bool b_digit = isPureDigit(uid_str);
  if (b_digit) {
    GetUserByUid(uid_str, rtvalue);
  } else {
    GetUserByName(uid_str, rtvalue);
  }
}

bool LogicSystem::isPureDigit(const std::string &str) {
  for (char c : str) {
    if (!std::isdigit(c)) {
      return false;
    }
  }
  return true;
}

void LogicSystem::GetUserByUid(std::string uid_str, Json::Value &rtvalue) {
  rtvalue["error"] = ErrorCodes::Success;

  std::string base_key = USER_BASE_INFO + uid_str;

  // 优先查redis中查询用户信息
  std::string info_str = "";
  bool b_base = RedisMgr::GetInstance()->Get(base_key, info_str);
  if (b_base) {
    Json::Reader reader;
    Json::Value root;
    reader.parse(info_str, root);
    auto uid = root["uid"].asInt();
    auto name = root["name"].asString();
    auto pwd = root["pwd"].asString();
    auto email = root["email"].asString();
    auto nick = root["nick"].asString();
    auto desc = root["desc"].asString();
    auto sex = root["sex"].asInt();
    auto icon = root["icon"].asString();
    LOG(info) << "user  uid is  " << uid << " name  is " << name << " pwd is "
              << pwd << " email is " << email << " icon is " << icon
              << std::endl;

    rtvalue["uid"] = uid;
    rtvalue["pwd"] = pwd;
    rtvalue["name"] = name;
    rtvalue["email"] = email;
    rtvalue["nick"] = nick;
    rtvalue["desc"] = desc;
    rtvalue["sex"] = sex;
    rtvalue["icon"] = icon;
    return;
  }

  auto uid = std::stoi(uid_str);
  // redis中没有则查询mysql
  // 查询数据库
  std::shared_ptr<UserInfo> user_info = nullptr;
  user_info = MysqlMgr::GetInstance()->GetUser(uid);
  if (user_info == nullptr) {
    rtvalue["error"] = ErrorCodes::UidInvalid;
    return;
  }

  // 将数据库内容写入redis缓存
  Json::Value redis_root;
  redis_root["uid"] = user_info->uid;
  redis_root["pwd"] = user_info->pwd;
  redis_root["name"] = user_info->name;
  redis_root["email"] = user_info->email;
  redis_root["nick"] = user_info->nick;
  redis_root["desc"] = user_info->desc;
  redis_root["sex"] = user_info->sex;
  redis_root["icon"] = user_info->icon;

  RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());

  // 返回数据
  rtvalue["uid"] = user_info->uid;
  rtvalue["pwd"] = user_info->pwd;
  rtvalue["name"] = user_info->name;
  rtvalue["email"] = user_info->email;
  rtvalue["nick"] = user_info->nick;
  rtvalue["desc"] = user_info->desc;
  rtvalue["sex"] = user_info->sex;
  rtvalue["icon"] = user_info->icon;
}

void LogicSystem::GetUserByName(std::string name, Json::Value &rtvalue) {
  rtvalue["error"] = ErrorCodes::Success;

  std::string base_key = NAME_INFO + name;

  // 优先查redis中查询用户信息
  std::string info_str = "";
  bool b_base = RedisMgr::GetInstance()->Get(base_key, info_str);
  if (b_base) {
    Json::Reader reader;
    Json::Value root;
    reader.parse(info_str, root);
    auto uid = root["uid"].asInt();
    auto name = root["name"].asString();
    auto pwd = root["pwd"].asString();
    auto email = root["email"].asString();
    auto nick = root["nick"].asString();
    auto desc = root["desc"].asString();
    auto sex = root["sex"].asInt();
    LOG(info) << "user  uid is  " << uid << " name  is " << name << " pwd is "
              << pwd << " email is " << email << std::endl;

    rtvalue["uid"] = uid;
    rtvalue["pwd"] = pwd;
    rtvalue["name"] = name;
    rtvalue["email"] = email;
    rtvalue["nick"] = nick;
    rtvalue["desc"] = desc;
    rtvalue["sex"] = sex;
    return;
  }

  // redis中没有则查询mysql
  // 查询数据库
  std::shared_ptr<UserInfo> user_info = nullptr;
  user_info = MysqlMgr::GetInstance()->GetUser(name);
  if (user_info == nullptr) {
    rtvalue["error"] = ErrorCodes::UidInvalid;
    return;
  }

  // 将数据库内容写入redis缓存
  Json::Value redis_root;
  redis_root["uid"] = user_info->uid;
  redis_root["pwd"] = user_info->pwd;
  redis_root["name"] = user_info->name;
  redis_root["email"] = user_info->email;
  redis_root["nick"] = user_info->nick;
  redis_root["desc"] = user_info->desc;
  redis_root["sex"] = user_info->sex;

  RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());

  // 返回数据
  rtvalue["uid"] = user_info->uid;
  rtvalue["pwd"] = user_info->pwd;
  rtvalue["name"] = user_info->name;
  rtvalue["email"] = user_info->email;
  rtvalue["nick"] = user_info->nick;
  rtvalue["desc"] = user_info->desc;
  rtvalue["sex"] = user_info->sex;
}

void LogicSystem::LoginHandler(std::shared_ptr<CSession> session,
                               const short &msg_id,
                               const std::string &msg_data) {
  Json::Reader reader;
  Json::Value root;
  reader.parse(msg_data, root);
  auto uid = root["uid"].asInt();
  auto token = root["token"].asString();
  LOG(info) << "user login uid is  " << uid << " user token  is " << token
            << std::endl;

  Json::Value rtvalue;
  Defer defer([&rtvalue, session]() {
    std::string return_str = rtvalue.toStyledString();
    session->Send(return_str, MSG_CHAT_LOGIN_RSP);
  });

  // 从redis获取用户token是否正确
  std::string uid_str = std::to_string(uid);
  std::string token_key = USERTOKENPREFIX + uid_str;
  std::string token_value = "";
  bool success = RedisMgr::GetInstance()->Get(token_key, token_value);
  if (!success) {
    rtvalue["error"] = ErrorCodes::UidInvalid;
    return;
  }

  if (token_value != token) {
    rtvalue["error"] = ErrorCodes::TokenInvalid;
    return;
  }

  rtvalue["error"] = ErrorCodes::Success;

  std::string base_key = USER_BASE_INFO + uid_str;
  auto user_info = std::make_shared<UserInfo>();
  bool b_base = GetBaseInfo(base_key, uid, user_info);
  if (!b_base) {
    rtvalue["error"] = ErrorCodes::UidInvalid;
    return;
  }
  rtvalue["uid"] = uid;
  rtvalue["pwd"] = user_info->pwd;
  rtvalue["name"] = user_info->name;
  rtvalue["email"] = user_info->email;
  rtvalue["nick"] = user_info->nick;
  rtvalue["desc"] = user_info->desc;
  rtvalue["sex"] = user_info->sex;
  rtvalue["icon"] = user_info->icon;

  // 从数据库获取申请列表

  // 获取好友列表

  auto server_name = ConfigMgr::Instance()["SelfServer"]["name"];
  // 将登录数量增加
  auto rd_res = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server_name);
  int count = 0;
  if (!rd_res.empty()) {
    count = std::stoi(rd_res);
  }

  count++;

  auto count_str = std::to_string(count);
  RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, count_str);

  // session绑定用户uid
  session->SetUserId(uid);

  // 为用户设置登录ip server的名字
  std::string ipkey = USERIPPREFIX + uid_str;
  RedisMgr::GetInstance()->Set(ipkey, server_name);

  // uid和session绑定管理,方便以后踢人操作
  LOG(info) << "user login uid is  " << uid << " user token  is " << token
            << std::endl;
  UserMgr::GetInstance()->SetUserSession(uid, session);

  return;
}