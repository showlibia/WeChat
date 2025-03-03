#include "LogicSystem.h"
#include "ConfigMgr.h"
#include "MysqlMgr.h"
#include "MysqlPool.h"
#include "RedisMgr.h"
#include "StatusGrpcClient.h"
#include "UserMgr.h"
#include "const.h"
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
  std::cout << __FUNCTION__ << std::endl;
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
        std::cout << "msg_id: " << msg_node->_recv_node->_msg_id << std::endl;
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
    std::cout << "recv_msg id  is " << msg_node->_recv_node->_msg_id
              << std::endl;
    auto call_back_iter = _fun_maps.find(msg_node->_recv_node->_msg_id);
    if (call_back_iter == _fun_maps.end()) {
      _msg_que.pop();
      std::cout << "msg id [" << msg_node->_recv_node->_msg_id
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
    std::cout << "user login uid is  " << userinfo->uid << " name  is "
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

void LogicSystem::LoginHandler(std::shared_ptr<CSession> session,
                               const short &msg_id,
                               const std::string &msg_data) {
  Json::Reader reader;
  Json::Value root;
  reader.parse(msg_data, root);
  auto uid = root["uid"].asInt();
  auto token = root["token"].asString();
  std::cout << "user login uid is  " << uid << " user token  is " << token
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
  UserMgr::GetInstance()->SetUserSession(uid, session);

  return;
}