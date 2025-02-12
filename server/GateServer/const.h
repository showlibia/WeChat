//
// Created by matrix on 2/6/25.
//

#ifndef SERVER_CONST_H
#define SERVER_CONST_H

#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http; // from <boost/beast/http.hpp>
namespace net = boost::asio; // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

enum ErrorCodes {
  Success = 0,
  Error_json = 1001,    // Json解析错误
  RPCFailed = 1002,     // RPC请求错误
  VerifyExpired = 1003, // 验证码过期
  VerifyCodeErr = 1004, // 验证码错误
  UserExist = 1005,     // 用户已经存在
};

#define VERIFY_CODE_PREFIX "code_"
#endif //SERVER_CONST_H
