//
// Created by 24183 on 2025/2/6.
//

#include "httpmgr.h"

void HttpMgr::PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod) {
    // 创建一个HTTP POST请求， 并设置请求头和请求体
    QByteArray data = QJsonDocument(json).toJson();
    // 通过url构造请求
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));

    // 利用shared_from_this()获取当前对象的shared_ptr, 构建伪闭包，防止对象提前析构
    auto self = shared_from_this();
    QNetworkReply * reply = _manager.post(request, data);
    connect(reply, &QNetworkReply::finished, [self, reply, req_id, mod]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            // 发送信号完成
            emit self->sig_http_finish(req_id, "", ERR_NETWORK, mod);
            reply->deleteLater();
            return;
        }

        // 无错误则读取请求
        QString res = reply->readAll();

        // 发送信号完成
        emit self->sig_http_finish(req_id, res, SUCCESS, mod);
        reply->deleteLater();
        return;
    });

}

HttpMgr::~HttpMgr() {

}

HttpMgr::HttpMgr() {
    // 连接http请求和完成信号，信号槽机制保证队列消费
    connect(this, &HttpMgr::sig_http_finish, this, &HttpMgr::slot_http_finish);
}

void HttpMgr::slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod) {
    if (mod == Modules::REGESTERMOD) {
        // 发送信号通知指定模块http响应结束
        emit sig_reg_mod_finish(id, res, err);
    }
}
