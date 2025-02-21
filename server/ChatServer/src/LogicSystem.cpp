#include "LogicSystem.h"
#include <memory>
#include <mutex>
#include <json/json.h>
#include "MysqlPool.h"
#include "StatusGrpcClient.h"
#include "const.h"
#include "value.h"
#include "MysqlMgr.h"

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
                                 std::string(msg_node->_recv_node->_data, msg_node->_recv_node->_cur_len));
          _msg_que.pop();
      }
      break;
    }

    // 如果没有停服，则说明队列中有数据
    auto msg_node = _msg_que.front();
    std::cout << "msg_id: " << msg_node->_recv_node->_msg_id << std::endl;
    auto call_back_iter = _fun_maps.find(msg_node->_recv_node->_msg_id);
    if (call_back_iter == _fun_maps.end()) {
      _msg_que.pop();
      continue;
    }
    call_back_iter->second(msg_node->_session, msg_node->_recv_node->_msg_id,
                           std::string(msg_node->_recv_node->_data,
                                       msg_node->_recv_node->_cur_len));
    _msg_que.pop();
  }
}

void LogicSystem::RegisterCallBacks() {
  _fun_maps[MSG_CHAT_LOGIN] = std::bind(&LogicSystem::LoginHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void LogicSystem::LoginHandler(std::shared_ptr<CSession> session,
                               const short &msg_id, const std::string &msg_data) {
  Json::Reader reader;
  Json::Value root;
  reader.parse(msg_data, root);
  auto uid = root["uid"].asInt();
  auto rsp =
      StatusGrpcClient::GetInstance()->Login(uid, root["token"].asString());
  Json::Value rtvalue;
  Defer defer([&rtvalue, session]() {
    std::string return_str = rtvalue.toStyledString();
    session->Send(return_str, MSG_CHAT_LOGIN_RSP);
  });

  rtvalue["error"] = rsp.error();
  if (rsp.error() != ErrorCodes::Success) {
    return;
  }

  auto find_iter = _users.find(uid);
  std::shared_ptr<UserInfo> user_info = nullptr;
  if (find_iter == _users.end()) {
    user_info = MysqlMgr::GetInstance()->GetUser(uid);
    if (user_info == nullptr) {
      rtvalue["error"] = ErrorCodes::UidInvalid;
      return;
    }

    _users[uid] = user_info;
  } else {
    user_info = find_iter->second;
  }

  rtvalue["uid"] = uid;
  rtvalue["name"] = user_info->name;
  rtvalue["token"] = rsp.token();
}