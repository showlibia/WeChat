#include "resetdialog.h"
#include "ui_resetdialog.h"
#include "httpmgr.h"
#include <QTimer>

ResetDialog::ResetDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ResetDialog)
{
    ui->setupUi(this);

    ui->newpass_edit->setEchoMode(QLineEdit::Password);

    _click_visible = new ClickVisible(this, ui->newpass_edit);

    ui->err_tip->setProperty("state", "normal");
    repolish(ui->err_tip);

    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reset_mod_finish, this, &ResetDialog::slot_reset_mod_finish);
    initHttpHandlers();

    ui->err_tip->clear();
    connect(ui->user_edit, &QLineEdit::editingFinished, this,[this](){
        checkUserValid();
    });

    connect(ui->email_edit, &QLineEdit::editingFinished, this,[this](){
        checkEmailValid();
    });

    connect(ui->newpass_edit, &QLineEdit::editingFinished, this, [this](){
        checkPassValid();
    });
    connect(ui->verify_edit, &QLineEdit::editingFinished, this, [this](){
        checkVerifyValid();
    });
}

ResetDialog::~ResetDialog()
{
    delete ui;
}

void ResetDialog::initHttpHandlers()
{
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

    // 注册重置密码逻辑
    _handlers.insert(ReqId::ID_RESET_PWD, [this](const QJsonObject& json){
        int err = json["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            showTip(tr("参数错误"), false);
            return;
        }
        auto email = json["email"].toString();
        showTip(tr("重置成功,点击返回登录"), true);
        qDebug() << "email is " << email;
        qDebug()<< "user uuid is " <<  json["uuid"].toString();

    });
}

bool ResetDialog::checkUserValid()
{
    auto user = ui->user_edit->text();
    if(user.isEmpty()) {
        AddTipErr(tr("用户名不能为空"), TipErr::TIP_USER_ERR);
        return false;
    }

    DelTipErr(TipErr::TIP_USER_ERR);
    return true;
}

bool ResetDialog::checkEmailValid()
{
    auto email = ui->email_edit->text();
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
    if(!match){
        AddTipErr(tr("邮箱地址不正确"), TipErr::TIP_EMAIL_ERR);
        return false;
    }
    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool ResetDialog::checkPassValid()
{
    auto pass = ui->newpass_edit->text();
    if(pass.length() < 6 || pass.length() > 15) {
        AddTipErr(tr("密码长度应为6~15"), TipErr::TIP_PWD_ERR);
        return false;
    }

    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
    bool match = regExp.match(pass).hasMatch();
    if(!match){
        //提示字符非法
        AddTipErr(tr("不能包含非法字符"), TipErr::TIP_PWD_ERR);
        return false;;
    }
    DelTipErr(TipErr::TIP_PWD_ERR);
    return true;
}

bool ResetDialog::checkVerifyValid()
{
    auto verify_code = ui->verify_edit->text();
    if(verify_code.isEmpty()) {
        AddTipErr(tr("验证码不能为空"), TipErr::TIP_VARIFY_ERR);
        return false;
    }
    DelTipErr(TipErr::TIP_VARIFY_ERR);
    return true;
}

void ResetDialog::AddTipErr(QString tips, TipErr te)
{
    _tip_errs[te] = tips;
    showTip(tips, false);
}

void ResetDialog::DelTipErr(TipErr te)
{
    _tip_errs.remove(te);
    if(_tip_errs.empty()) {
        ui->err_tip->clear();
        return;
    }

    showTip(_tip_errs.first(), false);
}

void ResetDialog::showTip(QString str, bool b_ok)
{
    if(b_ok) {
        ui->err_tip->setProperty("state", "normal");
    } else {
        ui->err_tip->setProperty("state", "err");
    }
    ui->err_tip->setText(str);

    repolish(ui->err_tip);
}

void ResetDialog::on_get_code_clicked()
{
    if (checkEmailValid()) {
        ui->get_code->startTimer();
        auto email = ui->email_edit->text();
        QJsonObject json_obj;
        json_obj["email"] = email;
        HttpMgr::GetInstance()->PostHttpReq(
            QUrl(gate_url_prefix + "/get_varifycode"), json_obj,
            ReqId::ID_GET_VERIFY_CODE, Modules::RESETMOD);
    }
}


void ResetDialog::on_confirm_btn_clicked()
{
    bool valid = checkUserValid();
    if(!valid){
        return;
    }
    valid = checkEmailValid();
    if(!valid){
        return;
    }
    valid = checkPassValid();
    if(!valid){
        return;
    }
    valid = checkVerifyValid();
    if(!valid){
        return;
    }

    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["passwd"] = md5Encrypt(ui->newpass_edit->text());
    json_obj["verifycode"] = ui->verify_edit->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/reset_pwd"),
                                        json_obj, ReqId::ID_REG_USER,Modules::RESETMOD);
}


void ResetDialog::on_return_btn_clicked()
{
    emit sigSwitchLogin();
}

void ResetDialog::slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err)
{
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

