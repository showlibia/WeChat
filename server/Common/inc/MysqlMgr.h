#ifndef SERVER_MYSQLMGR_H
#define SERVER_MYSQLMGR_H

#include "MysqlDao.h"
#include "Singleton.h"

class MysqlMgr : public Singleton<MysqlMgr> {
  friend class Singleton<MysqlMgr>;

public:
  ~MysqlMgr(){};
  int Register(const std::string &name, const std::string &password,
               const std::string &email);
  bool CheckEmail(const std::string &email, const std::string &name);
  bool UpdatePwd(const std::string &name, const std::string &password);
  bool CheckPwd(const std::string &email, const std::string &password, UserInfo &user_info);
  std::shared_ptr<UserInfo> GetUser(int uid);

private:
  MysqlMgr(){};
  MysqlDao _dao;
};

#endif // SERVER_MYSQLMGR_H