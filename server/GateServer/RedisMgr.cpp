//
// Created by matrix on 2/9/25.
//

#include "RedisMgr.h"
#include <cstring>

RedisMgr::RedisMgr() : _connection(nullptr), _reply(nullptr)
{}

RedisMgr::~RedisMgr() {
    _connection = nullptr;
    _reply = nullptr;
}

bool RedisMgr::Connect(const std::string &host, int port) {
    _connection = redisConnect(host.c_str(), port);
    if (_connection != nullptr && _connection->err) {
        std::cout << "Connect error " << _connection->errstr << std::endl;
        return false;
    }
    return true;
}

bool RedisMgr::Get(const std::string &key, std::string &value) {
    _reply = (redisReply*)redisCommand(_connection, "GET %s", key.c_str());
    if(_reply == nullptr) {
        std::cout << "[ GET " << key << " ] failed" << std::endl;
        freeReplyObject(_reply);
        return false;
    }

    if(_reply->type != REDIS_REPLY_STRING) {
        std::cout << "[ GET " << key << " ] failed" << std::endl;
        freeReplyObject(_reply);
        return false;
    }

    value = _reply->str;
    freeReplyObject(_reply);

    std::cout << "Succeed to execute command [ GET " << key << " ]" << std::endl;
    return true;
}

bool RedisMgr::Set(const std::string &key, const std::string &value) {
    _reply = (redisReply*)redisCommand(_connection, "SET %s %s", key.c_str(), value.c_str());
    if(_reply == nullptr) {
        std::cout << "[ SET " << key << " ] failed" << std::endl;
        freeReplyObject(_reply);
        return false;
    }

    if(!(_reply->type == REDIS_REPLY_STATUS && (std::strcmp(_reply->str, "OK") == 0 ||std::strcmp(_reply->str, "ok") == 0))) {
        std::cout << "[ SET " << key << " ] failed" << std::endl;
        freeReplyObject(_reply);
        return false;
    }

    freeReplyObject(_reply);

    std::cout << "Succeed to execute command [ SET " << key << " " << value << " ]" << std::endl;
    return true;
}

bool RedisMgr::Auth(const std::string &password) {
    _reply = (redisReply*)redisCommand(_connection, "AUTH %s", password.c_str());
    if(_reply->type == REDIS_REPLY_ERROR) {
        std::cout << "[ AUTH ] failed" << std::endl;
        freeReplyObject(_reply);
        return false;
    }
    freeReplyObject(_reply);
    std::cout << "Succeed to execute command [ AUTH ]" << std::endl;
    return true;
}

bool RedisMgr::LPush(const std::string &key, const std::string &value) {
    _reply = (redisReply*)redisCommand(_connection, "LPUSH %s %s", key.c_str(), value.c_str());
    if(_reply == nullptr) {
        std::cout << "[ LPUSH " << key << " ] failed" << std::endl;
        freeReplyObject(_reply);
        return false;
    }

    if(_reply->type != REDIS_REPLY_INTEGER || _reply->integer <= 0) {
        std::cout << "[ LPUSH " << key << " ] failed" << std::endl;
        freeReplyObject(_reply);
        return false;
    }

    freeReplyObject(_reply);
    std::cout << "Succeed to execute command [ LPUSH " << key << "  " << value << " ]" << std::endl;
    return true;
}

bool RedisMgr::LPop(const std::string &key, std::string &value) {
    _reply = (redisReply*)redisCommand(_connection, "LPOP %s", key.c_str());
    if(_reply == nullptr || _reply->type == REDIS_REPLY_NIL) {
        std::cout << "[ LPOP " << key << " ] failed" << std::endl;
        freeReplyObject(_reply);
        return false;
    }

    std::cout << "Succeed to execute command [ LPOP " << key << " ]" << std::endl;
    freeReplyObject(_reply);
    return true;
}

bool RedisMgr::RPush(const std::string &key, const std::string &value) {
    _reply = (redisReply*)redisCommand(_connection, "RPUSH %s %s", key.c_str(), value.c_str());
    if(_reply == nullptr) {
        std::cout << "[ RPUSH " << key << " ] failed" << std::endl;
        freeReplyObject(_reply);
        return false;
    }

    if(_reply->type != REDIS_REPLY_INTEGER || _reply->integer <= 0) {
        std::cout << "[ RPUSH " << key << " ] failed" << std::endl;
        freeReplyObject(_reply);
        return false;
    }

    freeReplyObject(_reply);
    std::cout << "Succeed to execute command [ RPUSH " << key << "  " << value << " ]" << std::endl;
    return true;
}

bool RedisMgr::RPop(const std::string &key, std::string &value) {
    _reply = (redisReply*)redisCommand(_connection, "RPOP %s", key.c_str());
    if(_reply == nullptr || _reply->type == REDIS_REPLY_NIL) {
        std::cout << "[ RPOP " << key << " ] failed" << std::endl;
        freeReplyObject(_reply);
        return false;
    }

    std::cout << "Succeed to execute command [ RPOP " << key << " ]" << std::endl;
    freeReplyObject(_reply);
    return true;
}

bool RedisMgr::HSet(const std::string &key, const std::string &hkey, const std::string &value) {
    _reply = (redisReply*) redisCommand(_connection, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
    if(_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "[ HSet " << key << " " << hkey << " " << value << " failed" << std::endl;
        freeReplyObject(_reply);
        return false;
    }
    std::cout << "Succeed to execute command [ HSet " << key << " " << hkey << " " << value << " ]" << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

bool RedisMgr::HSet(const char *key, const char *hkey, const char *hvalue, size_t hvaluelen) {
    const char * argv[4];
    size_t argvlen[4];
    argv[0] = "HSET";
    argvlen[0] = 4;
    argv[1] = key;
    argvlen[1] = strlen(key);
    argv[2] = hkey;
    argvlen[2] = strlen(hkey);
    argv[3] = hvalue;
    argvlen[3] = hvaluelen;
    _reply = (redisReply*) redisCommandArgv(_connection, 4, argv, argvlen);
    if(_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "[ HSet " << key << " " << hkey << " " << hvalue << " failed" << std::endl;
        freeReplyObject(_reply);
        return false;
    }
    std::cout << "Succeed to execute command [ HSet " << key << " " << hkey << " " << hvalue << " ]" << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

std::string RedisMgr::HGet(const std::string &key, const std::string &hkey) {
    const char* argv[3];
    size_t argvlen[3];
    argv[0] = "HGET";
    argvlen[0] = 4;
    argv[1] = key.c_str();
    argvlen[1] = key.length();
    argv[2] = hkey.c_str();
    argvlen[2] = hkey.length();
    _reply = (redisReply*) redisCommandArgv(_connection, 3, argv, argvlen);
    if(_reply == nullptr || _reply->type == REDIS_REPLY_NIL) {
        std::cout << "[ HGet " << key << " " << hkey << " ] failed" << std::endl;
        freeReplyObject(_reply);
        return "";
    }

    std::string value = _reply->str;
    freeReplyObject(_reply);
    std::cout << "Succeed to execute command [ HGet " << key << " " << hkey << " ]" << std::endl;
    return value;
}

bool RedisMgr::Del(const std::string &key) {
    _reply = (redisReply*) redisCommand(_connection, "DEL %s", key.c_str());
    if(_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "[ DEL " << key << " failed" << std::endl;
        freeReplyObject(_reply);
        return false;
    }
    std::cout << "Succeed to execute command [ DEL " << key << " ]" << std::endl;
    freeReplyObject(_reply);
    return true;
}

bool RedisMgr::ExistsKey(const std::string &key) {
    _reply = (redisReply*) redisCommand(_connection, "EXISTS %s", key.c_str());
    if(_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER || _reply->integer == 0) {
        std::cout << "Not Found [ Key " << key << " ]" << std::endl;
        freeReplyObject(_reply);
        return false;
    }
    std::cout << "Found [ Key " << key << " ]" << std::endl;
    freeReplyObject(_reply);
    return true;
}

void RedisMgr::Close() {
    redisFree(_connection);
}

