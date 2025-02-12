//
// Created by matrix on 2/10/25.
//

#include "RedisConPool.h"
#include <iostream>

RedisConPool::RedisConPool(std::size_t pool_size, const std::string &host,
                           int port, const char *password)
    : _b_stop(false) , _pool_size(pool_size), _host(host), _port(port) {
  for (size_t i = 0; i < _pool_size; ++i) {
    auto connection = redisConnect(_host.c_str(), _port);
    if (connection == nullptr || connection->err != 0) {
      if (connection != nullptr) {
        redisFree(connection);
      }
      continue;
    }
    auto reply = (redisReply *)redisCommand(connection, "AUTH %s", password);
    if (reply == nullptr) {
      std::cerr << "认证命令执行失败: 无返回" << std::endl;
      redisFree(connection);
      continue;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
      std::cerr << "认证失败: " << reply->str << std::endl;
      freeReplyObject(reply);
      redisFree(connection);
      continue;
    }

    freeReplyObject(reply);
    std::cout << "认证成功" << std::endl;
    _connections.push(std::unique_ptr<redisContext>(connection));
  }
}

void RedisConPool::Close() {
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _b_stop = true;
  }
  _cv.notify_all();
  // 等待所有连接归还
}

RedisConPool::~RedisConPool() {
  std::cout << __FUNCTION__ << std::endl;
  Close();
  std::lock_guard<std::mutex> lock(_mutex);
  while (!_connections.empty()) {
    auto connection = std::move(_connections.front());
    redisFree(connection.release());
    _connections.pop();
  }
}

std::shared_ptr<redisContext> RedisConPool::GetConnection() {
  std::unique_lock<std::mutex> lock(_mutex);
  _cv.wait(lock, [this]() {
    if (_b_stop) {
      return true;
    }
    return !_connections.empty();
  });
  if (_b_stop) {
    return nullptr;
  }

  auto connection = std::move(_connections.front());
  _connections.pop();

  return std::shared_ptr<redisContext>(
      connection.release(), [this](redisContext *ptr) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_b_stop) {
          lock.unlock();
          redisFree(ptr);
        } else {
          _connections.push(std::unique_ptr<redisContext>(ptr));
          lock.unlock();
          _cv.notify_one();
        }
        _cv.notify_all();
      });
}