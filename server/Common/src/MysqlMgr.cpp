#include "MysqlMgr.h"

int MysqlMgr::Register(const std::string &name, const std::string &password,
                       const std::string &email) {
  return _dao.Register(name, password, email);
}

bool MysqlMgr::CheckEmail(const std::string &email, const std::string &name) {
  return _dao.CheckEmail(email, name);
}

bool MysqlMgr::UpdatePwd(const std::string &name, const std::string &password) {
  return _dao.UpdatePwd(name, password);
}

bool MysqlMgr::CheckPwd(const std::string &email, const std::string &password, UserInfo &user_info) {
  return _dao.CheckPwd(email, password, user_info);
}

std::shared_ptr<UserInfo> MysqlMgr::GetUser(int uid) {
  return _dao.GetUser(uid);
}

std::shared_ptr<UserInfo> MysqlMgr::GetUser(std::string name) {
  return _dao.GetUser(name);
}

bool MysqlMgr::AddFriendApply(const int &from, const int &to) {
  return _dao.AddFriendApply(from, to);
}

bool MysqlMgr::AuthFriendApply(const int &from, const int &to) {
  return _dao.AuthFriendApply(from, to);
}

bool MysqlMgr::AddFriend(const int &from, const int &to,
                         std::string back_name) {
  return _dao.AddFriend(from, to, back_name);
}

bool MysqlMgr::GetApplyList(int touid,
                            std::vector<std::shared_ptr<ApplyInfo>> &applyList,
                            int begin, int limit) {

  return _dao.GetApplyList(touid, applyList, begin, limit);
}

bool MysqlMgr::GetFriendList(
    int self_id, std::vector<std::shared_ptr<UserInfo>> &user_info) {
  return _dao.GetFriendList(self_id, user_info);
}
