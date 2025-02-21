#include "CServer.h"
#include "MsgNode.h"
#include <boost/asio/write.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstddef>
#include <cstring>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>

CSession::CSession(net::io_context &ioc, CServer *server)
    : _socket(ioc), _server(server), _b_close(false), _b_head_parse(false) {
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  _uuid = boost::uuids::to_string(uuid);
  _recv_head_node = std::make_shared<MsgNode>(HEAD_TOTAL_LEN);
}

CSession::~CSession() {}

tcp::socket &CSession::GetSocket() { return _socket; }

std::string &CSession::GetUuid() { return _uuid; }

void CSession::Start() { AsyncReadHead(HEAD_TOTAL_LEN); }

void CSession::Send(std::string msg, short msgid) {
  std::lock_guard<std::mutex> lock(_send_mutex);
  int send_que_size = _send_que.size();
  if (send_que_size > MAX_SENDQUE) {
    std::cout << "Session: " << _uuid << " send queue is full" << std::endl;
    return;
  }

  _send_que.push(std::make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
  if (send_que_size > 0) {
    return;
  }

  auto &msg_node = _send_que.front();
  boost::asio::async_write(
      _socket, boost::asio::buffer(msg_node->_data, msg_node->_total_len),
      std::bind(&CSession::HandleWrite, this, std::placeholders::_1,
                SharedSelf()));
}

void CSession::Send(char *msg, short max_length, short msgid) {
  std::lock_guard<std::mutex> lock(_send_mutex);
  int send_que_size = _send_que.size();
  if (send_que_size > MAX_SENDQUE) {
    std::cout << "Session: " << _uuid << " send queue is full" << std::endl;
    return;
  }

  _send_que.push(std::make_shared<SendNode>(msg, max_length, msgid));
  if (send_que_size > 0) {
    return;
  }
  auto &msg_node = _send_que.front();
  boost::asio::async_write(
      _socket, boost::asio::buffer(msg_node->_data, msg_node->_total_len),
      std::bind(&CSession::HandleWrite, this, std::placeholders::_1,
                SharedSelf()));
}

void CSession::Close() {
  _socket.close();
  _b_close = true;
}

std::shared_ptr<CSession> CSession::SharedSelf() { return shared_from_this(); }

void CSession::AsyncReadBody(int total_len) {
  auto self = shared_from_this();
  // readfull
  asyncReadFull(total_len, [self, total_len,
                            this](const boost::system::error_code &ec,
                                  std::size_t bytes_transferred) {
    try {
      if (ec) {
        std::cerr << "handle read failed, error is " << ec.what() << std::endl;
        Close();
        _server->ClearSession(_uuid);
        return;
      }

      if (bytes_transferred < total_len) {
        std::cout << "read length not match, read [" << bytes_transferred
                  << "] , total [" << total_len << "]" << std::endl;
        Close();
        _server->ClearSession(_uuid);
        return;
      }

      ::memcpy(_recv_msg_node->_data, _data, bytes_transferred);
      _recv_msg_node->_cur_len += bytes_transferred;
      _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
      std::cout << "receive data is " << _recv_msg_node->_data << std::endl;
      // 然后将消息放入逻辑队列
      
      AsyncReadHead(HEAD_TOTAL_LEN);
    } catch (std::exception &e) {
      std::cerr << "Exception: " << e.what() << std::endl;
    }
  });
}

void CSession::AsyncReadHead(int total_len) {
  auto self = shared_from_this();
  asyncReadFull(HEAD_TOTAL_LEN, [self,
                                 this](const boost::system::error_code &ec,
                                       std::size_t bytes_transferred) {
    try {
      if (ec) {
        std::cerr << "handle read failed, error is " << ec.what() << std::endl;
        Close();
          _server->ClearSession(_uuid);
        return;
      }

      if (bytes_transferred < HEAD_TOTAL_LEN) {
        std::cout << "read length not match, read [" << bytes_transferred
                  << "] , total [" << HEAD_TOTAL_LEN << "]" << std::endl;
        Close();
          _server->ClearSession(_uuid);
        return;
      }

      _recv_head_node->Clear();
      ::memcpy(_recv_head_node->_data, _data, bytes_transferred);

      short msgid = 0;
      memcpy(&msgid, _recv_head_node->_data, HEAD_ID_LEN);
      msgid = boost::asio::detail::socket_ops::network_to_host_short(msgid);
      if (msgid > MAX_LENGTH) {
        std::cerr << "invaid msgid is " << msgid << std::endl;
        _server->ClearSession(_uuid);
        return;
      }
      short msglen = 0;
      memcpy(&msglen, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
      msglen = boost::asio::detail::socket_ops::network_to_host_short(msglen);
      if (msglen > MAX_LENGTH) {
        std::cerr << "invaid msglen is " << msglen << std::endl;
        _server->ClearSession(_uuid);
        return;
      }
      _recv_msg_node = std::make_shared<RecvNode>(msglen, msgid);
      AsyncReadBody(msglen);

    } catch (std::exception &e) {
      std::cerr << "Exception: " << e.what() << std::endl;
    }
  });
}

void CSession::HandleWrite(const boost::system::error_code &error,
                           std::shared_ptr<CSession> shared_self) {
  try {
    if (error) {
      std::cerr << "write failed, error is " << error.what() << std::endl;
      Close();
      _server->ClearSession(_uuid);
      return;
    } else {
      std::lock_guard<std::mutex> lock(_send_mutex);
      // 由于是异步发送，不能在Send中调用完HandleWrite后直接pop
      // 而是需要在HandleWrite中pop, 保证发送的顺序
      _send_que.pop();
      if (!_send_que.empty()) {
        auto &msg_node = _send_que.front();
        boost::asio::async_write(
            _socket, boost::asio::buffer(msg_node->_data, msg_node->_total_len),
            std::bind(&CSession::HandleWrite, this, std::placeholders::_1,
                      shared_self));
      }
    }
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

void CSession::asyncReadFull(
    std::size_t max_length,
    std::function<void(const boost::system::error_code &, std::size_t)>
        handler) {
  ::memset(_data, 0, MAX_LENGTH);
  asyncReadLen(0, max_length, handler);
}

void CSession::asyncReadLen(
    std::size_t read_len, std::size_t total_len,
    std::function<void(const boost::system::error_code &, std::size_t)>
        handler) {
  auto self = shared_from_this();
  _socket.async_read_some(
      boost::asio::buffer(_data + read_len, total_len - read_len),
      [read_len, total_len, handler, self](const boost::system::error_code &ec,
                                           std::size_t bytes_transferred) {
        if (ec) {
          handler(ec, read_len + bytes_transferred);
          return;
        }

        if (read_len + bytes_transferred >= total_len) {
          handler(ec, read_len + bytes_transferred);
          return;
        }
        self->asyncReadLen(read_len + bytes_transferred, total_len, handler);
      });
}

LogicNode::LogicNode(std::shared_ptr<CSession> session,
                     std::shared_ptr<RecvNode> msg_node)
    : _session(session), _recv_node(msg_node) {}