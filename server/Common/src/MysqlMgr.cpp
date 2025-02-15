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