//
// Created by matrix on 2/7/25.
//

#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"

void LogicSystem::RegGet(const std::string& url, const HttpHandler& handler) {
    _get_handlers.insert(std::make_pair(url, handler));
}

void LogicSystem::RegPost(const std::string &url, const HttpHandler & handler) {
    _post_handlers.insert(std::make_pair(url, handler));
}

LogicSystem::LogicSystem() {
    RegGet("/get_test", [](const std::shared_ptr<HttpConnection>& connect){
        beast::ostream(connect->_response.body()) << "received get_test request" << std::endl;
        int i = 0;
        for (auto & elem: connect->_get_params) {
            ++i;
            beast::ostream(connect->_response.body()) << "param " << i << ": " << elem.first << " = " << elem.second << std::endl;
        }
    });

    RegPost("/get_verifycode", [](const std::shared_ptr<HttpConnection>& connect){
        auto body_str = beast::buffers_to_string(connect->_request.body().cdata());
        std::cout << "receive body is " << body_str;
        connect->_response.set(http::field::content_type, "text/json");
        Json::Value root; // 发送给客户端的json数据
        Json::Reader reader; // 将json反序列化为对象
        Json::Value src_root; // 客户端发送过来的json数据
        bool parse_success = reader.parse(body_str, src_root);
        if(!parse_success) {
            std::cout << "Failed to parse JSON data!" << std::endl;
            root["error"] = ErrorCodes::Error_json;
            std::string jsonStr = root.toStyledString();
            beast::ostream(connect->_response.body()) << jsonStr;
            return true;
        }

        if(!src_root.isMember("email")) {
            std::cout << "Failed to parse JSON data!" << std::endl;
            root["error"] = ErrorCodes::Error_json;
            std::string jsonStr = root.toStyledString();
            beast::ostream(connect->_response.body()) << jsonStr;
            return true;
        }
        auto email = src_root["email"].asString();
        GetVerifyRsp rps = VerifyGrpcClient::GetInstance()->GetVerifyCode(email);
        std::cout << "email is " << email << std::endl;
        root["error"] = rps.error();
        root["email"] = email;
        std::string jsonStr = root.toStyledString();
        beast::ostream(connect->_response.body()) << jsonStr;
        return true;
    });
}

bool LogicSystem::HandleGet(const std::string& path, const std::shared_ptr<HttpConnection> &connection) {
    if(_get_handlers.find(path) == _get_handlers.end()) {
        return false;
    }

    _get_handlers[path](connection);
    return true;
}

bool LogicSystem::HandlePost(const std::string & path, const std::shared_ptr<HttpConnection> & connection) {
    if (_post_handlers.find(path) == _post_handlers.end()) {
        return false;
    }

    _post_handlers[path](connection);
    return true;
}
