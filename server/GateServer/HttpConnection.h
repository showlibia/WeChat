//
// Created by matrix on 2/6/25.
//

#ifndef SERVER_HTTPCONNECTION_H
#define SERVER_HTTPCONNECTION_H

#include "const.h"

class HttpConnection : public std::enable_shared_from_this<HttpConnection>{
public:
    friend class LogicSystem;
    explicit HttpConnection(tcp::socket socket);
    void Start();

private:
    void CheckDeadline();
    void WriteResponse();
    void HandleReq();
    void PreParseGetParam();
    tcp::socket _socket;
    beast::flat_buffer _buffer{8192};
    http::request<http::dynamic_body> _request;
    http::response<http::dynamic_body> _response;
    net::steady_timer _deadline{
        _socket.get_executor(), std::chrono::seconds(60)
    };

    std::string _get_url;
    std::unordered_map<std::string, std::string> _get_params;
};


#endif //SERVER_HTTPCONNECTION_H
