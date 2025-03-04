//
// Created by matrix on 2/6/25.
//

#ifndef SERVER_CONST_H
#define SERVER_CONST_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http; // from <boost/beast/http.hpp>
namespace net = boost::asio; // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

enum ErrorCodes {
  Success = 0,
  Error_json = 1001,     // Json解析错误
  RPCFailed = 1002,      // RPC请求错误
  VerifyExpired = 1003,  // 验证码过期
  VerifyCodeErr = 1004,  // 验证码错误
  UserExist = 1005,      // 用户已经存在
  EmailNotMatch = 1006,  // 邮箱不匹配
  PasswdUpFailed = 1007, // 更新密码失败
  LoginInvalid = 1008,   // 密码更新失败
  TokenInvalid = 1009,   // Token失效
  UidInvalid = 1010,     // uid无效
};

// 定义最大消息长度为2KB
const int MAX_LENGTH = 1024 * 2;
const int MAX_SENDQUE = 1000;
const int MAX_RECVQUE = 10000;
const int HEAD_TOTAL_LEN = 4;
const int HEAD_ID_LEN = 2;
const int HEAD_DATA_LEN = 2;

enum MSG_IDS {
  MSG_CHAT_LOGIN = 1005,              // 登录
  MSG_CHAT_LOGIN_RSP = 1006,          // 用户登陆回包
  ID_SEARCH_USER_REQ = 1007,          // 用户搜索请求
  ID_SEARCH_USER_RSP = 1008,          // 搜索用户回包
  ID_ADD_FRIEND_REQ = 1009,           // 申请添加好友请求
  ID_ADD_FRIEND_RSP = 1010,           // 申请添加好友回复
  ID_NOTIFY_ADD_FRIEND_REQ = 1011,    // 通知用户添加好友申请
  ID_AUTH_FRIEND_REQ = 1013,          // 认证好友请求
  ID_AUTH_FRIEND_RSP = 1014,          // 认证好友回复
  ID_NOTIFY_AUTH_FRIEND_REQ = 1015,   // 通知用户认证好友申请
  ID_TEXT_CHAT_MSG_REQ = 1017,        // 文本聊天信息请求
  ID_TEXT_CHAT_MSG_RSP = 1018,        // 文本聊天信息回复
  ID_NOTIFY_TEXT_CHAT_MSG_REQ = 1019, // 通知用户文本聊天信息
};

#define VERIFY_CODE_PREFIX "code_"
#define USERIPPREFIX "uip_"
#define USERTOKENPREFIX "utoken_"
#define IPCOUNTPREFIX "ipcount_"
#define USER_BASE_INFO "ubaseinfo_"
#define LOGIN_COUNT "logincount"
#define NAME_INFO "nameinfo_"

#endif //SERVER_CONST_H
