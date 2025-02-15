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

private:
  MysqlMgr(){};
  MysqlDao _dao;
};

#endif // SERVER_MYSQLMGR_H