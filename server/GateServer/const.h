//
// Created by matrix on 2/6/25.
//

#ifndef SERVER_CONST_H
#define SERVER_CONST_H

#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <iostream>
#include <map>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>


namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http; // from <boost/beast/http.hpp>
namespace net = boost::asio; // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

enum ErrorCodes {
    Success = 0,
    Error_json = 1001, // json解析错误
    RPCFailed = 1002, // rpc调用失败
};

class ConfigMgr;
extern ConfigMgr gCfgMgr;

#endif //SERVER_CONST_H
