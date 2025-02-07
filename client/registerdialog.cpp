#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "httpmgr.h"
#include "global.h"

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);

    // 设置密码隐藏
    ui->confirm_edit->setEchoMode(QLineEdit::Password);
    ui->pass_edit->setEchoMode(QLineEdit::Password);

    ui->err_tip->setProperty("state", "normal");
    repolish(ui->err_tip);

    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);
    initHttpHandlers();
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_get_code_clicked()
{
    auto email = ui->email_edit->text();
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch();
    if(match) {
        // 发送http请求获取验证码
        QJsonObject json_obj;
        json_obj["email"] = email;
        HttpMgr::GetInstance()->PostHttpReq(
            QUrl(gate_url_prefix + "/get_verifycode"),
            json_obj,
            ReqId::ID_GET_VERIFY_CODE,
            Modules::REGESTERMOD
            );
    } else {
        showTip(tr("邮箱地址不正确"), false);
    }
}

void RegisterDialog::showTip(QString str, bool b_ok)
{
    if(b_ok) {
        ui->err_tip->setProperty("state", "normal");
    } else {
        ui->err_tip->setProperty("state", "err");
    }
    ui->err_tip->setText(str);

    repolish(ui->err_tip);
}

void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err) {
    if(err != ErrorCodes::SUCCESS) {
        showTip(tr("网络请求错误"), false);
        return;
    }

    // 解析JSON字符串, res需转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    // json解析错误
    if(jsonDoc.isNull()) {
        showTip(tr("json解析错误"), false);
        return;
    }
    if (!jsonDoc.isObject()) {
        showTip(tr("json格式错误"), false);
        return;
    }
    QJsonObject jsonObj = jsonDoc.object();

    _handlers[id](jsonObj);
    return;
}

void RegisterDialog::initHttpHandlers() {
    _handlers.insert(ReqId::ID_GET_VERIFY_CODE, [this](const QJsonObject& json){
        // 处理获取验证码的返回
        int err = json["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            showTip(tr("参数错误"), false);
            return;
        }
        auto email = json["email"].toString();
        showTip(tr("验证码已发送至邮箱"), true);
        qDebug() << "email is " << email;
    });
}
