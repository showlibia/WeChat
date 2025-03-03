#ifndef SERVER_CSERVER_H
#define SERVER_CSERVER_H

#include "CSession.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <map>
#include <memory>
#include <mutex>

using boost::asio::ip::tcp;

class CServer {
public:
  CServer(boost::asio::io_context &ioc, short port);
  ~CServer();
  void ClearSession(std::string);

private:
  void HandleAccept(std::shared_ptr<CSession>, const boost::system::error_code &);
  void StartAccept();
  boost::asio::io_context &_ioc;
  short _port;
  tcp::acceptor _acceptor;
  std::map<std::string, std::shared_ptr<CSession>> _sessions;
  std::mutex _mutex;
};

#endif //SERVER_CSERVER_H

