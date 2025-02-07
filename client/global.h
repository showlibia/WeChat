#ifndef GLOBAL_H
#define GLOBAL_H

#include <QWidget>
#include <QRegularExpression>
#include <functional>
#include <iostream>
#include <mutex>
#include <memory>
#include "QStyle"
#include <QSettings>
#include <QDir>

extern QString gate_url_prefix;

/**
 * @brief repolish 用来刷新qss
 */
extern std::function<void(QWidget*)> repolish;

enum ReqId {
    ID_GET_VERIFY_CODE = 1001, // 获取验证码
    ID_REG_USER = 1002, // 注册用户
};

enum Modules {
    REGESTERMOD = 0,
};

enum ErrorCodes {
    SUCCESS = 0,
    ERR_JSON = 1, // json解析错误
    ERR_NETWORK = 2, // 网络错误
};

#endif // GLOBAL_H
