#ifndef SERVER_MSGNODE_H
#define SERVER_MSGNODE_H

#include <boost/asio.hpp>
#include <cstring>

using boost::asio::ip::tcp;

class MsgNode {
public:
  MsgNode(short max_len);
  ~MsgNode();
  void Clear();
  short _cur_len;
  short _total_len;
  char *_data;
};

class RecvNode : public MsgNode {
  friend class LogicSystem;

public:
  RecvNode(short max_len, short msg_id);

private:
  short _msg_id;
};

class SendNode : public MsgNode {
  friend class LogicSystem;

public:
  SendNode(const char *data, short max_len, short msg_id);

private:
  short _msg_id;
};

#endif // SERVER_MSGNODE_H