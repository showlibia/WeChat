#include "MsgNode.h"
#include "const.h"
#include <boost/asio/detail/socket_ops.hpp>
#include <cstring>

MsgNode::MsgNode(short max_len) : _cur_len(0), _total_len(max_len) {
  _data = new char[_total_len + 1];
  _data[_total_len] = '\0';
}

MsgNode::~MsgNode() { delete[] _data; }

void MsgNode::Clear() {
  _cur_len = 0;
  ::memset(_data, 0, _total_len);
}

RecvNode::RecvNode(short max_len, short msg_id)
    : MsgNode(max_len), _msg_id(msg_id) {}

SendNode::SendNode(const char *data, short max_len, short msg_id)
    : MsgNode(max_len + HEAD_TOTAL_LEN), _msg_id(msg_id) {
  short msg_id_host =
      boost::asio::detail::socket_ops::host_to_network_short(msg_id);
  ::memcpy(_data, &msg_id_host, HEAD_ID_LEN);
  short max_len_host =
      boost::asio::detail::socket_ops::host_to_network_short(max_len);
  ::memcpy(_data + HEAD_ID_LEN, &max_len_host, HEAD_DATA_LEN);
  ::memcpy(_data + HEAD_TOTAL_LEN, data, max_len);
}