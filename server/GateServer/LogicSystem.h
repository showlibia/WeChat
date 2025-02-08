//
// Created by matrix on 2/7/25.
//

#ifndef SERVER_LOGICSYSTEM_H
#define SERVER_LOGICSYSTEM_H

#include "Singleton.h"
#include "const.h"
#include <functional>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>


class HttpConnection;
using HttpHandler = std::function<void(std::shared_ptr<HttpConnection>)>;
class LogicSystem : public Singleton<LogicSystem> {
    // 设置为友元类，以便访问私有构造函数
    friend class Singleton<LogicSystem>;
public:
    ~LogicSystem() = default;
    bool HandleGet(const std::string&, const std::shared_ptr<HttpConnection> &);
    bool HandlePost(const std::string&, const std::shared_ptr<HttpConnection> &);
    void RegGet(const std::string&, const HttpHandler&);
    void RegPost(const std::string&, const HttpHandler&);
private:
    LogicSystem();
    std::map<const std::string, HttpHandler> _post_handlers;
    std::map<const std::string, HttpHandler> _get_handlers;
};


#endif //SERVER_LOGICSYSTEM_H
