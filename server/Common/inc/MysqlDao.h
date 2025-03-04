#ifndef SERVER_MYSQLDAO_H
#define SERVER_MYSQLDAO_H

#include "MysqlPool.h"
#include <memory>
#include "data.h"

class MysqlDao {
public:
  MysqlDao();
  ~MysqlDao() = default;
  int Register(const std::string &name, const std::string &password,
               const std::string &email);
  bool CheckEmail(const std::string &email, const std::string &name);
  bool UpdatePwd(const std::string &name, const std::string &password);
  bool CheckPwd(const std::string &email, const std::string &password,
                UserInfo &user_info);
  std::shared_ptr<UserInfo> GetUser(int uid);
  std::shared_ptr<UserInfo> GetUser(std::string name);
  bool AddFriendApply(const int &from, const int &to);
  bool AuthFriendApply(const int &from, const int &to);
  bool AddFriend(const int &from, const int &to, std::string back_name);
  bool GetApplyList(int touid,
                    std::vector<std::shared_ptr<ApplyInfo>> &applyList,
                    int offset, int limit);
  bool GetFriendList(int self_id,
                     std::vector<std::shared_ptr<UserInfo>> &user_info);

private:
  std::unique_ptr<MysqlPool> _pool;
};

#endif // SERVER_MYSQLDAO_H