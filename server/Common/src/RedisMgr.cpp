//
// Created by matrix on 2/9/25.
//

#include "RedisMgr.h"
#include "ConfigMgr.h"
#include "Logger.h"
#include <boost/log/trivial.hpp>
#include <cstring>
#include <memory>

RedisMgr::RedisMgr() {
  auto &gCfgMgr = ConfigMgr::Instance();
  std::string host = gCfgMgr["Redis"]["host"];
  std::string port = gCfgMgr["Redis"]["port"];
  std::string password = gCfgMgr["Redis"]["password"];
  _pool = std::make_unique<RedisConPool>(5, host, std::stoi(port),
                                         password.c_str());
}

void RedisMgr::Close() { _pool->Close(); }

RedisMgr::~RedisMgr() { Close(); }

bool RedisMgr::Get(const std::string &key, std::string &value) {
  auto connection = _pool->GetConnection();
  if (connection == nullptr) {
    LOG(warning) << "[GET " << key << "] 无可用连接" << std::endl;
    return false;
  }
  RedisReplyPtr reply(static_cast<redisReply *>(
      redisCommand(connection.get(), "GET %s", key.c_str())));
  if (reply == nullptr) {
    LOG(warning) << "[ GET " << key << " ] failed" << std::endl;
    return false;
  }

  if (reply->type != REDIS_REPLY_STRING) {
    LOG(warning) << "[ GET " << key << " ] failed" << std::endl;
    return false;
  }

  value = reply->str;

  LOG(info) << "Succeed to execute command [ GET " << key << " ]" << std::endl;
  return true;
}

bool RedisMgr::Set(const std::string &key, const std::string &value) {
  auto connection = _pool->GetConnection();
  if (connection == nullptr) {
    LOG(warning) << "[SET " << key << "] 无可用连接" << std::endl;
    return false;
  }
  RedisReplyPtr reply(static_cast<redisReply *>(
      redisCommand(connection.get(), "SET %s %s", key.c_str(), value.c_str())));
  if (reply == nullptr) {
    LOG(warning) << "[ SET " << key << " ] failed" << std::endl;
    return false;
  }

  if (!(reply->type == REDIS_REPLY_STATUS &&
        (std::strcmp(reply->str, "OK") == 0 ||
         std::strcmp(reply->str, "ok") == 0))) {
    LOG(warning) << "[ SET " << key << " ] failed" << std::endl;
    return false;
  }

  LOG(info) << "Succeed to execute command [ SET " << key << " " << value
            << " ]" << std::endl;
  return true;
}

bool RedisMgr::Auth(const std::string &password) {
  auto connection = _pool->GetConnection();
  if (connection == nullptr) {
    return false;
  }
  RedisReplyPtr reply(static_cast<redisReply *>(
      redisCommand(connection.get(), "AUTH %s", password.c_str())));

  if (reply == nullptr) {
    LOG(warning) << "[ AUTH ] failed (no response)" << std::endl;
    return false;
  }

  if (reply->type == REDIS_REPLY_ERROR) {
    LOG(warning) << "[ AUTH ] failed" << std::endl;
    return false;
  }
  LOG(info) << "Succeed to execute command [ AUTH ]" << std::endl;
  return true;
}

bool RedisMgr::LPush(const std::string &key, const std::string &value) {
  auto connection = _pool->GetConnection();
  if (connection == nullptr) {
    return false;
  }
  RedisReplyPtr reply(static_cast<redisReply *>(redisCommand(
      connection.get(), "LPUSH %s %s", key.c_str(), value.c_str())));
  if (reply == nullptr) {
    LOG(warning) << "[ LPUSH " << key << " ] failed" << std::endl;
    return false;
  }

  if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
    LOG(warning) << "[ LPUSH " << key << " ] failed" << std::endl;
    return false;
  }

  LOG(info) << "Succeed to execute command [ LPUSH " << key << "  " << value
            << " ]" << std::endl;
  return true;
}

bool RedisMgr::LPop(const std::string &key, std::string &value) {
  auto connection = _pool->GetConnection();
  if (connection == nullptr) {
    return false;
  }
  RedisReplyPtr reply(static_cast<redisReply *>(
      redisCommand(connection.get(), "LPOP %s", key.c_str())));
  if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
    LOG(warning) << "[ LPOP " << key << " ] failed" << std::endl;
    return false;
  }

  if (reply->type == REDIS_REPLY_STRING) {
    value = reply->str;
  }
  LOG(info) << "Succeed to execute command [ LPOP " << key << " ]" << std::endl;
  return true;
}

bool RedisMgr::RPush(const std::string &key, const std::string &value) {
  auto connection = _pool->GetConnection();
  if (connection == nullptr) {
    return false;
  }
  RedisReplyPtr reply(static_cast<redisReply *>(redisCommand(
      connection.get(), "RPUSH %s %s", key.c_str(), value.c_str())));
  if (reply == nullptr) {
    LOG(warning) << "[ RPUSH " << key << " ] failed" << std::endl;
    return false;
  }

  if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
    LOG(warning) << "[ RPUSH " << key << " ] failed" << std::endl;
    return false;
  }

  LOG(info) << "Succeed to execute command [ RPUSH " << key << "  " << value
            << " ]" << std::endl;
  return true;
}

bool RedisMgr::RPop(const std::string &key, std::string &value) {
  auto connection = _pool->GetConnection();
  if (connection == nullptr) {
    return false;
  }
  RedisReplyPtr reply(static_cast<redisReply *>(
      redisCommand(connection.get(), "RPOP %s", key.c_str())));
  if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
    LOG(warning) << "[ RPOP " << key << " ] failed" << std::endl;
    return false;
  }

  if (reply->type == REDIS_REPLY_STRING) {
    value = reply->str;
  }
  LOG(info) << "Succeed to execute command [ RPOP " << key << " ]" << std::endl;
  return true;
}

bool RedisMgr::HSet(const std::string &key, const std::string &hkey,
                    const std::string &value) {
  auto connection = _pool->GetConnection();
  if (connection == nullptr) {
    return false;
  }
  RedisReplyPtr reply(static_cast<redisReply *>(
      redisCommand(connection.get(), "HSET %s %s %s", key.c_str(), hkey.c_str(),
                   value.c_str())));
  if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
    LOG(info) << "[ HSet " << key << " " << hkey << " " << value << " failed"
              << std::endl;
    return false;
  }
  LOG(info) << "Succeed to execute command [ HSet " << key << " " << hkey << " "
            << value << " ]" << std::endl;
  return true;
}

bool RedisMgr::HSet(const char *key, const char *hkey, const char *hvalue,
                    size_t hvaluelen) {
  auto connection = _pool->GetConnection();
  if (connection == nullptr) {
    return false;
  }
  const char *argv[4];
  size_t argvlen[4];
  argv[0] = "HSET";
  argvlen[0] = 4;
  argv[1] = key;
  argvlen[1] = strlen(key);
  argv[2] = hkey;
  argvlen[2] = strlen(hkey);
  argv[3] = hvalue;
  argvlen[3] = hvaluelen;
  RedisReplyPtr reply(static_cast<redisReply *>(
      redisCommandArgv(connection.get(), 4, argv, argvlen)));
  if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
    LOG(warning) << "[ HSet " << key << " " << hkey << " " << hvalue
                 << " failed" << std::endl;
    return false;
  }
  LOG(info) << "Succeed to execute command [ HSet " << key << " " << hkey << " "
            << hvalue << " ]" << std::endl;
  return true;
}

std::string RedisMgr::HGet(const std::string &key, const std::string &hkey) {
  auto connection = _pool->GetConnection();
  if (connection == nullptr) {
    return "";
  }
  const char *argv[3];
  size_t argvlen[3];
  argv[0] = "HGET";
  argvlen[0] = 4;
  argv[1] = key.c_str();
  argvlen[1] = key.length();
  argv[2] = hkey.c_str();
  argvlen[2] = hkey.length();
  RedisReplyPtr reply(static_cast<redisReply *>(
      redisCommandArgv(connection.get(), 3, argv, argvlen)));
  if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
    LOG(warning) << "[ HGet " << key << " " << hkey << " ] failed" << std::endl;
    return "";
  }

  std::string value;
  if (reply->type == REDIS_REPLY_STRING && reply->str) {
    value = reply->str;
  }
  LOG(info) << "Succeed to execute command [ HGet " << key << " " << hkey
            << " ]" << std::endl;
  return value;
}

bool RedisMgr::Del(const std::string &key) {
  auto connection = _pool->GetConnection();
  if (connection == nullptr) {
    return false;
  }
  RedisReplyPtr reply(static_cast<redisReply *>(
      redisCommand(connection.get(), "DEL %s", key.c_str())));
  if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
    LOG(warning) << "[ DEL " << key << " ] failed" << std::endl;
    return false;
  }
  LOG(info) << "Succeed to execute command [ DEL " << key << " ]" << std::endl;
  return true;
}

bool RedisMgr::ExistsKey(const std::string &key) {
  auto connection = _pool->GetConnection();
  if (connection == nullptr) {
    return false;
  }
  RedisReplyPtr reply(static_cast<redisReply *>(
      redisCommand(connection.get(), "EXISTS %s", key.c_str())));
  if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER ||
      reply->integer == 0) {
    LOG(warning) << "Not Found [ Key " << key << " ]" << std::endl;
    return false;
  }
  LOG(info) << "Found [ Key " << key << " ]" << std::endl;
  return true;
}

bool RedisMgr::HDel(const std::string &key, const std::string &field) {
  auto connection = _pool->GetConnection();
  if (connection == nullptr) {
    return false;
  }
  RedisReplyPtr reply(static_cast<redisReply *>(redisCommand(
      connection.get(), "HDEL %s %s", key.c_str(), field.c_str())));
  if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
    LOG(warning) << "[ HDEL " << key << " " << field << " ] failed"
                 << std::endl;
    return false;
  }
  LOG(info) << "Succeed to execute command [ HDEL " << key << " " << field
            << " ]" << std::endl;
  return true;
}