#ifndef SERVER_LOGINSYSTEM_H
#define SERVER_LOGINSYSTEM_H

#include "CServer.h"
#include "MysqlDao.h"
#include "Singleton.h"
#include <condition_variable>
#include <functional>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
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
  void SearchInfo(std::shared_ptr<CSession> session, const short &msg_id,
                  const std::string &msg_data);
  void AddFriendApply(std::shared_ptr<CSession> session, const short &msg_id,
                      const std::string &msg_data);
  void AuthFriendApply(std::shared_ptr<CSession> session, const short &msg_id,
                       const std::string &msg_data);
  void DealChatTextMsg(std::shared_ptr<CSession> session, const short &msg_id,
                       const std::string &msg_data);
  bool isPureDigit(const std::string &str);
  void GetUserByUid(std::string uid_str, Json::Value &rtvalue);
  void GetUserByName(std::string name, Json::Value &rtvalue);
  bool GetBaseInfo(std::string base_key, int uid,
                   std::shared_ptr<UserInfo> &userinfo);
  bool GetFriendApplyInfo(int to_uid,
                          std::vector<std::shared_ptr<ApplyInfo>> &list);
  bool GetFriendList(int self_id,
                     std::vector<std::shared_ptr<UserInfo>> &user_list);
  std::thread _work;
  std::queue<std::shared_ptr<LogicNode>> _msg_que;
  std::mutex _mutex;
  std::condition_variable _consume;
  bool _b_stop;
  std::map<short, FunCallBack> _fun_maps;
};

#endif // SERVER_LOGINSYSTEM_H