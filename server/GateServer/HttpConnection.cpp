//
// Created by matrix on 2/6/25.
//

#include "HttpConnection.h"
#include "LogicSystem.h"

HttpConnection::HttpConnection(tcp::socket socket)
    : _socket(std::move(socket))
{}

void HttpConnection::Start() {
    auto self = shared_from_this();
    http::async_read(_socket, _buffer, _request, [self](beast::error_code ec, std::size_t bytes_transferred){
        try {
            if (ec) {
                std::cout << "http read err is " << ec.what() << std::endl;
                return ;
            }

            // 处理读取到的数据
            boost::ignore_unused(bytes_transferred);
            self->HandleReq();
            self->CheckDeadline();
        } catch (std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    });
}

//char 转为16进制
unsigned char ToHex(unsigned char x)
{
    return  x > 9 ? x + 55 : x + 48;
}

//16进制转为char
unsigned char FromHex(unsigned char x)
{
    unsigned char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else assert(0);
    return y;
}

std::string UrlEncode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        //判断是否仅有数字和字母构成
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ') //为空字符
            strTemp += "+";
        else
        {
            //其他字符需要提前加%并且高四位和低四位分别转为16进制
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] & 0x0F);
        }
    }
    return strTemp;
}

std::string UrlDecode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        //还原+为空
        if (str[i] == '+') strTemp += ' ';
            //遇到%将后面的两个字符从16进制转为char再拼接
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            strTemp += high * 16 + low;
        }
        else strTemp += str[i];
    }
    return strTemp;
}

void HttpConnection::PreParseGetParam() {
    // http://localhost:8080/?name=matrix&age=18
    // 提取url
    auto url = _request.target();
    // 查找查询字符串的开始位置(?)
    auto query_pos = url.find("?");
    if(query_pos == std::string::npos) {
        _get_url = url;
        return;
    }
    _get_url = url.substr(0, query_pos);
    std::string query_str = url.substr(query_pos + 1);
    // name=matrix&age=18
    std::string key;
    std::string value;
    size_t pos = 0;
    while((pos = query_str.find("&")) != std::string::npos) {
        auto pair = query_str.substr(0, pos);
        auto equal_pos = pair.find("=");
        if(equal_pos != std::string::npos) {
            key = UrlDecode(pair.substr(0, equal_pos));
            value = UrlDecode(pair.substr(equal_pos + 1));
            _get_params[key] = value;
        }
        query_str.erase(0, pos + 1);
    }
    if(!query_str.empty()) {
        auto equal_pos = query_str.find("=");
        if(equal_pos != std::string::npos) {
            key = UrlDecode(query_str.substr(0, equal_pos));
            value = UrlDecode(query_str.substr(equal_pos + 1));
            _get_params[key] = value;
        }
    }
}

void HttpConnection::HandleReq() {
    // 设置版本
    _response.version(_request.version());
    // 设置为短连接
    _response.keep_alive(false);

    if(_request.method() == http::verb::get) {
        PreParseGetParam();
        bool success = LogicSystem::GetInstance()->HandleGet(_get_url, shared_from_this());
        if(!success) {
            _response.result(http::status::not_found);
            _response.set(http::field::content_type, "text/plain");
            beast::ostream(_response.body()) << "url not found\r\n";
            WriteResponse();
            return;
        }

        _response.result(http::status::ok);
        _response.set(http::field::server, "GateServer");

        WriteResponse();
        return;
    }
    if(_request.method() == http::verb::post) {
        bool success = LogicSystem::GetInstance()->HandlePost(_request.target(), shared_from_this());
        if(!success) {
            _response.result(http::status::not_found);
            _response.set(http::field::content_type, "text/plain");
            beast::ostream(_response.body()) << "url not found\r\n";
            WriteResponse();
            return;
        }

        _response.result(http::status::ok);
        _response.set(http::field::server, "GateServer");

        WriteResponse();
        return;
    }
}

void HttpConnection::WriteResponse() {
    auto self = shared_from_this();
    _response.content_length(_response.body().size());
    http::async_write(_socket, _response, [self](beast::error_code ec, std::size_t bytes_transferred){
        self->_socket.shutdown(tcp::socket::shutdown_send, ec);
        self->_deadline.cancel();
    });
}

void HttpConnection::CheckDeadline() {
    auto self = shared_from_this();

    _deadline.async_wait([self](beast::error_code ec){
        if(!ec) {
            self->_socket.close(ec);
        }
    });
}