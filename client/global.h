#ifndef GLOBAL_H
#define GLOBAL_H

#include <QWidget>
#include <QRegularExpression>
#include <functional>
#include <iostream>
#include <mutex>
#include <memory>
#include <QStyle>
#include <QSettings>
#include <QDir>

extern QString gate_url_prefix;
extern QString md5Encrypt(const QString& password);

/**
 * @brief repolish 用来刷新qss
 */
extern std::function<void(QWidget*)> repolish;

enum ReqId {
    ID_GET_VERIFY_CODE = 1001, //获取验证码
    ID_REG_USER = 1002, //注册用户
    ID_RESET_PWD = 1003, //重置密码
    ID_LOGIN_USER = 1004, //用户登录
    ID_CHAT_LOGIN = 1005, //登陆聊天服务器
    ID_CHAT_LOGIN_RSP= 1006, //登陆聊天服务器回包
};

enum Modules {
    REGISTERMOD = 0,
    RESETMOD = 1,
    LOGINMOD = 2,
};

enum ErrorCodes {
    SUCCESS = 0,
    ERR_JSON = 1, // json解析错误
    ERR_NETWORK = 2, // 网络错误
};

enum TipErr{
    TIP_SUCCESS = 0,
    TIP_EMAIL_ERR = 1,
    TIP_PWD_ERR = 2,
    TIP_CONFIRM_ERR = 3,
    TIP_PWD_CONFIRM = 4,
    TIP_VARIFY_ERR = 5,
    TIP_USER_ERR = 6
};

enum ClickLbState{
    Normal = 0,
    Selected = 1
};

#endif // GLOBAL_H
