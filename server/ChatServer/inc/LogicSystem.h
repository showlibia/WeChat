#ifndef SERVER_LOGINSYSTEM_H
#define SERVER_LOGINSYSTEM_H

#include "CServer.h"
#include "MysqlDao.h"
#include "Singleton.h"
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
using FunCallBack = std::function<void(std::shared_ptr<CSession>, const short& msg_id, const std::string &msg_data)>;

class LogicSystem : public Singleton<LogicSystem> {
  friend class Singleton<LogicSystem>;

public:
  ~LogicSystem();
  void PostMsgToQue(std::shared_ptr<LogicNode> msg);

private:
  LogicSystem();
  void DealMsg();
  void RegisterCallBacks();
  void LoginHandler(std::shared_ptr<CSession> session, const short &msg_id,
                    const std::string &msg_data);
  std::thread _work;
  std::queue<std::shared_ptr<LogicNode>> _msg_que;
  std::mutex _mutex;
  std::condition_variable _consume;
  bool _b_stop;
  std::map<short, FunCallBack> _fun_maps;
  std::unordered_map<int, std::shared_ptr<UserInfo>> _users;
};

#endif // SERVER_LOGINSYSTEM_H