#include "tcpmgr.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "usermgr.h"

TcpMgr::~TcpMgr()
{

}

TcpMgr::TcpMgr(): _host(""), _port(0), _b_recv_pending(false), _message_id(0), _message_length(0)
{
    QObject::connect(&_socket, &QTcpSocket::connected, [&](){
        qDebug() << "Connected to server";
        emit sig_con_success(true);
    });

    QObject::connect(&_socket, &QTcpSocket::readyRead, [&](){
        // 当有数据可读时，读取所有数据
        _buffer.append(_socket.readAll());

        QDataStream stream(&_buffer, QIODevice::ReadOnly);


        forever {
            // 解析头部
            if(!_b_recv_pending) {
                // 检查在缓冲区是否能读出一个消息头（id + len)
                if (_buffer.size() < static_cast<int>(sizeof(quint16) * 2)) {
                    return; // 数据不够，等待更多数据
                }

                stream >> _message_id >> _message_length;

                _buffer = _buffer.mid(sizeof(quint16)*2);

                qDebug() << "Message ID:" << _message_id << ", Length:" << _message_length;
            }

            if(_buffer.size() < _message_length) {
                qDebug() << "Reading message's buffer is not enough";
                _b_recv_pending = true;
                return;
            }

            _b_recv_pending = false;

            QByteArray messageBody = _buffer.mid(0, _message_length);
            qDebug() << "receive body msg is " << messageBody ;

            _buffer = _buffer.mid(_message_length);
            handleMsg(ReqId(_message_id), _message_length, messageBody);
        }
    });

    QObject::connect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), [&](QAbstractSocket::SocketError socketError){
        qDebug() << "Error:" << _socket.errorString();
        switch (socketError) {
        case QTcpSocket::ConnectionRefusedError:
            qDebug() << "Connection Refused!";
            emit sig_con_success(false);
            break;
        case QTcpSocket::RemoteHostClosedError:
            qDebug() << "Remote Host Closed Connection!";
            break;
        case QTcpSocket::HostNotFoundError:
            qDebug() << "Host Not Found!";
            emit sig_con_success(false);
            break;
        case QTcpSocket::SocketTimeoutError:
            qDebug() << "Connection Timeout!";
            emit sig_con_success(false);
            break;
        case QTcpSocket::NetworkError:
            qDebug() << "Network Error!";
            break;
        default:
            qDebug() << "Other Error!";
            break;
        }
    });

    // 处理连接断开
    QObject::connect(&_socket, &QTcpSocket::disconnected, [&]() {
        qDebug() << "Disconnected from server.";
    });
    QObject::connect(this, &TcpMgr::sig_send_data, this, &TcpMgr::slot_send_data);
    // 注册消息
    initHandlers();
}

void TcpMgr::initHandlers()
{
    _handlers.insert(ReqId::ID_CHAT_LOGIN_RSP, [this](ReqId id, int len, const QByteArray &data){
        qDebug()<< "handle id is "<< id << " data is " << data;

        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        if(jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if(!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Login Failed, err is Json Parse Err" << err ;
            emit sig_login_failed(err);
            return;
        }

        int err = jsonObj["error"].toInt();

        if(err != ErrorCodes::SUCCESS) {
            qDebug() << "Login Failed, err is " << err ;
            emit sig_login_failed(err);
            return;
        }

        auto uid = jsonObj["uid"].toInt();
        auto name = jsonObj["name"].toString();
        auto nick = jsonObj["nick"].toString();
        auto icon = jsonObj["icon"].toString();
        auto sex = jsonObj["sex"].toInt();
        auto user_info = std::make_shared<UserInfo>(uid, name, nick, icon, sex);

        UserMgr::GetInstance()->SetUserInfo(user_info);
        UserMgr::GetInstance()->SetToken(jsonObj["token"].toString());
        UserMgr::GetInstance()->SetToken(jsonObj["token"].toString());
        emit sig_swich_chatdlg();
    });
}

void TcpMgr::handleMsg(ReqId id, int len, QByteArray data)
{
    auto find = _handlers.find(id);

    if(find == _handlers.end()) {
        qDebug()<< "not found id ["<< id << "] to handle";
        return ;
    }

    find.value()(id, len, data);
}

void TcpMgr::slot_tcp_connect(ServerInfo si)
{
    qDebug()<< "receive tcp connect signal";
    // 尝试连接到服务器
    qDebug() << "Connecting to server...";

    _host = si.Host;
    _port = static_cast<uint16_t>(si.Port.toInt());
    _socket.connectToHost(_host, _port);
}

void TcpMgr::slot_send_data(ReqId reqId, QByteArray dataBytes)
{
    uint16_t id = reqId;

    // 计算长度（使用网络字节序转换）
    quint16 len = static_cast<quint16>(dataBytes.length());

    // 创建一个QByteArray用于存储要发送的所有数据
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    // 设置数据流使用网络字节序
    out.setByteOrder(QDataStream::BigEndian);

    // 写入ID和长度
    out << id << len;

    qDebug() << "Sending message - ID:" << id << "Length:" << len;

    // 添加字符串数据
    block.append(dataBytes);

    // 发送数据
    _socket.write(block);
    qDebug() << "tcp mgr send byte data is " << block ;
}
