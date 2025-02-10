//
// Created by matrix on 2/10/25.
//

#ifndef SERVER_REDISCONPOOL_H
#define SERVER_REDISCONPOOL_H

#include "hiredis.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>

class RedisConPool {
public:
  RedisConPool(std::size_t pool_size, const std::string &host, int port,
               const char *password);
  ~RedisConPool();
  std::shared_ptr<redisContext> GetConnection();
  void Close();

private:
  std::atomic_bool _b_stop;
  std::size_t _pool_size;
  std::string _host;
  int _port;
  std::queue<std::unique_ptr<redisContext>> _connections;
  std::mutex _mutex;
  std::condition_variable _cv;

  // 追踪正在使用的连接数量
  std::atomic<size_t> _in_use{0};

  // 辅助函数：等待所有连接归还
  void WaitForConnectionsToReturn();
};


#endif //SERVER_REDISCONPOOL_H
