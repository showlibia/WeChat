#include "MysqlMgr.h"

int MysqlMgr::Register(const std::string &name, const std::string &password,
                       const std::string &email) {
  return _dao.Register(name, password, email);
}