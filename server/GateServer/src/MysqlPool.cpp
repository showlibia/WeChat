#include "MysqlPool.h"
#include <chrono>
#include <iostream>
#include <mariadb/conncpp/Driver.hpp>
#include <mariadb/conncpp/Exception.hpp>
#include <mariadb/conncpp/Statement.hpp>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

MysqlPool::MysqlPool(const std::string &url, const std::string &user,
                     const std::string &password, const std::string &schema,
                     int pool_size)
    : _url(url), _user(user), _password(password), _schema(schema),
      _pool_size(pool_size), _b_stop(false) {
  try {
    for (int i = 0; i < _pool_size; ++i) {
      sql::Driver *driver = sql::mariadb::get_driver_instance();
      auto connection = driver->connect(_url, _user, _password);
      connection->setSchema(_schema);
      // 获取当前时间戳
      auto currentTime = std::chrono::system_clock::now().time_since_epoch();
      // 将当前时间戳转换为秒
      long long timestamp =
          std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
      _pool.push(std::make_unique<SqlConnection>(connection, timestamp));

      // 启动一个线程，检查连接是否超时
      _check_thread = std::thread([this]() {
        while (!_b_stop) {
          checkConnection();
          std::this_thread::sleep_for(std::chrono::seconds(60));
        }
      });

      _check_thread.detach();
    }
  } catch (sql::SQLException &e) {
    std::cerr << "mysql pool init failed: " << e.what() << std::endl;
  }
}

void MysqlPool::checkConnection() {
  std::lock_guard<std::mutex> lock(_mutex);
  int pool_size = _pool.size();
  auto currentTime = std::chrono::system_clock::now().time_since_epoch();
  long long timestamp =
      std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
  for (int i = 0; i < pool_size; ++i) {
    auto connection = std::move(_pool.front());
    _pool.pop();

    Defer Defer([this, &connection]() { _pool.push(std::move(connection)); });

    if (timestamp - connection->_last_oper_time < 5) {
      continue;
    }

    try {
      std::unique_ptr<sql::Statement> stmt(
          connection->_connection->createStatement());
      stmt->execute("SELECT 1");
      connection->_last_oper_time = timestamp;
    } catch (sql::SQLException &e) {
      std::cerr << "Error keep connection alive: " << e.what() << std::endl;
      // 创建新连接并替换旧连接
      sql::Driver *driver = sql::mariadb::get_driver_instance();
      auto new_connection = driver->connect(_url, _user, _password);
      new_connection->setSchema(_schema);
      connection->_connection.reset(new_connection);
      connection->_last_oper_time = timestamp;
    }
  }
}

std::shared_ptr<SqlConnection> MysqlPool::GetConnection() {
  std::unique_lock<std::mutex> lock(_mutex);
  _cv.wait(lock, [this]() {
    if (_b_stop) {
      return true;
    }
    return !_pool.empty();
  });
  if (_b_stop) {
    return nullptr;
  }

  auto connection = std::move(_pool.front());
  _pool.pop();
  return std::shared_ptr<SqlConnection>(
      connection.release(), [this](SqlConnection *connection) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_b_stop) {
          lock.unlock();
        } else {
          _pool.push(std::unique_ptr<SqlConnection>(connection));
          lock.unlock();
          _cv.notify_one();
        }
        _cv.notify_all();
    });
}

MysqlPool::~MysqlPool() {
  std::cout << __FUNCTION__ << std::endl;
  Close();
  std::lock_guard<std::mutex> lock(_mutex);
  while (!_pool.empty()) {
    auto connection = std::move(_pool.front());
    connection->_connection->close();
    _pool.pop();
  }
}

void MysqlPool::Close() {
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _b_stop = true;
  }
  _cv.notify_all();
}