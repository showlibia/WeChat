//
// Created by matrix on 2/9/25.
//

#ifndef SERVER_REDISMGR_H
#define SERVER_REDISMGR_H

#include "RedisConPool.h"
#include "Singleton.h"

// 自定义删除器，用于自动释放 redisReply 对象
struct RedisReplyDeleter {
  void operator()(redisReply *reply) const {
    if (reply)
      freeReplyObject(reply);
  }
};
using RedisReplyPtr = std::unique_ptr<redisReply, RedisReplyDeleter>;

class RedisMgr : public Singleton<RedisMgr>,
                 public std::enable_shared_from_this<RedisMgr> {
  friend class Singleton<RedisMgr>;

public:
  ~RedisMgr();

  bool Get(const std::string &key, std::string &value);
  bool Set(const std::string &key, const std::string &value);
  bool Auth(const std::string &password);
  bool LPush(const std::string &key, const std::string &value);
  bool LPop(const std::string &key, std::string &value);
  bool RPush(const std::string &key, const std::string &value);
  bool RPop(const std::string &key, std::string &value);
  bool HSet(const std::string &key, const std::string &hkey,
            const std::string &value);
  bool HSet(const char *key, const char *hkey, const char *hvalue,
            size_t hvaluelen);
  std::string HGet(const std::string &key, const std::string &hkey);
  bool Del(const std::string &key);
  bool HDel(const std::string &key, const std::string &field);
  bool ExistsKey(const std::string &key);
  void Close();

private:
  RedisMgr();

  std::unique_ptr<RedisConPool> _pool;
};

#endif // SERVER_REDISMGR_H
