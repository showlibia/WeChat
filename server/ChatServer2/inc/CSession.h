#include "MsgNode.h"
#include "const.h"
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/uuid/uuid.hpp>
#include <cstddef>
#include <functional>
#include <memory>
#include <queue>

namespace beast = boost::beast;   // 来自 <boost/beast.hpp>
namespace http = beast::http;     // 来自 <boost/beast/http.hpp>
namespace net = boost::asio;      // 来自 <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // 来自 <boost/asio/ip/tcp.hpp>

class CServer;
/**
 * @brief 会话类，处理单个客户端连接的通信
 * 继承自 enable_shared_from_this 以支持共享指针管理
 */
class CSession : public std::enable_shared_from_this<CSession> {
public:
  // 构造函数，需要IO上下文和服务器指针
  CSession(net::io_context &ioc, CServer *server);
  ~CSession();

  // 获取底层Socket对象
  tcp::socket &GetSocket();
  std::string &GetSessionId();
  void SetUserId(int uid);
  int GetUserId();

  /**
   * 开始一个新的会话
   * 异步读取消息头部信息，长度为HEAD_TOTAL_LEN
   */
  void Start();

  // 发送消息的两个重载函数
  void Send(char *msg, short max_length, short msgid);
  void Send(std::string msg, short msgid);

  // 关闭会话连接
  void Close();

  // 获取当前对象的共享指针
  std::shared_ptr<CSession> SharedSelf();

  // 异步读取消息体和消息头
  void AsyncReadBody(int length);
  void AsyncReadHead(int total_len);

private:
  // 异步读取完整消息
  void asyncReadFull(
      std::size_t max_length,
      std::function<void(const boost::system::error_code &, std::size_t)>
          handler);

  // 异步读取指定长度的数据
  void asyncReadLen(
      std::size_t read_len, std::size_t total_len,
      std::function<void(const boost::system::error_code &, std::size_t)>
          handler);

  // 处理写操作完成后的回调
  void HandleWrite(const boost::system::error_code &error,
                   std::shared_ptr<CSession> shared_self);

  tcp::socket _socket;                             // 网络通信套接字
  std::string _session_id;                         // 会话唯一标识符
  char _data[MAX_LENGTH];                          // 数据缓冲区
  CServer *_server;                                // 服务器指针
  bool _b_close;                                   // 关闭标志
  std::queue<std::shared_ptr<SendNode>> _send_que; // 发送消息队列
  std::mutex _send_mutex;                          // 发送队列互斥锁

  std::shared_ptr<RecvNode> _recv_msg_node; // 接收消息节点
  bool _b_head_parse;                       // 头部解析标志
  std::shared_ptr<MsgNode> _recv_head_node; // 接收到的消息头节点
  int _user_uid;
};

class LogicNode {
  friend class LogicSystem;

public:
  LogicNode(std::shared_ptr<CSession> session,
            std::shared_ptr<RecvNode> msg_node);

private:
  std::shared_ptr<CSession> _session;
  std::shared_ptr<RecvNode> _recv_node;
};