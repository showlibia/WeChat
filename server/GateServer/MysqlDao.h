#ifndef SERVER_MYSQLDAO_H
#define SERVER_MYSQLDAO_H

#include "MysqlPool.h"
#include <memory>

struct UserInfo {
  std::string name;
  std::string password;
  std::string email;
  int uid;
};

class MysqlDao {
public:
  MysqlDao();
  ~MysqlDao() = default;
  int Register(const std::string &name, const std::string &password,
               const std::string &email);
private:
  std::unique_ptr<MysqlPool> _pool;
};

#endif // SERVER_MYSQLDAO_H