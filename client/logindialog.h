#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "clickvisible.h"
#include "global.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private:
    void initHead();
    void initHttpHandlers();
    bool checkEmailValid();
    bool checkPassValid();
    void AddTipErr(QString tips, TipErr te);
    void DelTipErr(TipErr te);
    void showTip(QString str, bool b_ok);
    bool enableBtn(bool);

    Ui::LoginDialog *ui;
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    QMap<TipErr, QString> _tip_errs;
    ClickVisible* _visible;
    int _uid;
    QString _token;
private slots:
    // 重置密码
    void slot_forget_pwd();
    // 登录模块Button槽函数
    void on_login_btn_clicked();
    // 登录模块完成槽函数
    void slot_login_mod_finish(ReqId id, QString res, ErrorCodes err);
    void slot_tcp_con_finish(bool bsuccess);
    void slot_login_failed(int);
signals:
    void switchRegister();
    void switchReset();
    void sig_connect_tcp(ServerInfo);
};

#endif // LOGINDIALOG_H
