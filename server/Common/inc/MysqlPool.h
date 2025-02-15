#ifndef SERVER_MYSQLPOOL_H
#define SERVER_MYSQLPOOL_H

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mariadb/conncpp.hpp>
#include <mariadb/conncpp/Connection.hpp>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

class SqlConnection {
public:
  SqlConnection(sql::Connection *connection, int64_t last_time): _connection(connection), _last_oper_time(last_time) {};
  std::unique_ptr<sql::Connection> _connection;
  int64_t _last_oper_time;
};

class Defer {
public:
  Defer(std::function<void()> func) : _func(func) {}
  ~Defer() { _func(); }

private:
  std::function<void()> _func;
};

class MysqlPool {
public:
  MysqlPool(const std::string &url, const std::string &user, const std::string &password, const std::string &schema, int pool_size);
  void checkConnection();
  std::shared_ptr<SqlConnection> GetConnection();
  ~MysqlPool();
  void Close();
private:
  std::string _url;
  std::string _user;
  std::string _password;
  std::string _schema;
  int _pool_size;
  std::queue<std::unique_ptr<SqlConnection>> _pool;
  std::mutex _mutex;
  std::condition_variable _cv;
  std::atomic_bool _b_stop;
  std::thread _check_thread;
};

#endif // SERVER_MYSQLPOOL_H