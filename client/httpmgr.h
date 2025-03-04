//
// Created by 24183 on 2025/2/6.
//

#ifndef WECHAT_HTTPMGR_H
#define WECHAT_HTTPMGR_H

#include "singleton.h"
#include "global.h"
#include <QJsonObject>
#include <QUrl>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class HttpMgr: public QObject, public Singleton<HttpMgr>, public std::enable_shared_from_this<HttpMgr>{
    Q_OBJECT
public:
    ~HttpMgr();
    void PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod);
private:
    friend class Singleton<HttpMgr>;
    HttpMgr();
    QNetworkAccessManager _manager;
public slots:
    void slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
signals:
    void sig_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
    //注册模块http相关请求完成发送此信号
    void sig_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
    // 重置密码模块信号
    void sig_reset_mod_finish(ReqId id, QString res, ErrorCodes err);
    // 登录模块信号
    void sig_login_mod_finish(ReqId id, QString res, ErrorCodes err);
};


#endif //WECHAT_HTTPMGR_H
