#include "logindialog.h"
#include "ui_logindialog.h"
#include <QPixmap>
#include <QPainterPath>
#include <QPainter>
#include <QJsonObject>
#include <httpmgr.h>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    connect(ui->register_button, &QPushButton::clicked, this, &LoginDialog::switchRegister);
    ui->forget_label->SetState("normal", "hover", "", "selected", "seleced_hover", "");
    ui->forget_label->setCursor(Qt::PointingHandCursor);

    _visible = new ClickVisible(this, ui->pass_edit);

    ui->err_tip->setProperty("state", "normal");
    repolish(ui->err_tip);
    ui->pass_edit->setEchoMode(QLineEdit::Password);

    initHead();
    initHttpHandlers();

    connect(ui->forget_label, &ClickLabel::clicked, this, &LoginDialog::slot_forget_pwd);
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_login_mod_finish, this, &LoginDialog::slot_login_mod_finish);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::initHead()
{
    // 加载图片
    QPixmap originalPixmap(":/res/head_1.jpg");
        // 设置图片自动缩放
    originalPixmap = originalPixmap.scaled(ui->head_label->size(),
                                           Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 创建一个和原始图片相同大小的QPixmap，用于绘制圆角图片
    QPixmap roundedPixmap(originalPixmap.size());
    roundedPixmap.fill(Qt::transparent); // 用透明色填充

    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing); // 设置抗锯齿，使圆角更平滑
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 使用QPainterPath设置圆角
    QPainterPath path;
    path.addRoundedRect(0, 0, originalPixmap.width(), originalPixmap.height(), 10, 10); // 最后两个参数分别是x和y方向的圆角半径
    painter.setClipPath(path);

    // 将原始图片绘制到roundedPixmap上
    painter.drawPixmap(0, 0, originalPixmap);

    // 设置绘制好的圆角图片到QLabel上
    ui->head_label->setPixmap(roundedPixmap);
}

void LoginDialog::initHttpHandlers()
{
    _handlers.insert(ReqId::ID_LOGIN_USER, [this](const QJsonObject& json){
        // 处理获取验证码的返回
        int err = json["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            showTip(tr("参数错误"), false);
            return;
        }
        auto email = json["email"].toString();
        showTip(tr("登录成功"), true);
        qDebug() << "[LOGIN] email is " << email;
    });
}

bool LoginDialog::checkEmailValid()
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

bool LoginDialog::checkPassValid()
{
    auto pass = ui->pass_edit->text();
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

void LoginDialog::AddTipErr(QString tips, TipErr te)
{
    _tip_errs[te] = tips;
    showTip(tips, false);
}

void LoginDialog::DelTipErr(TipErr te)
{
    _tip_errs.remove(te);
    if(_tip_errs.empty()) {
        ui->err_tip->clear();
        return;
    }

    showTip(_tip_errs.first(), false);
}

void LoginDialog::showTip(QString str, bool b_ok)
{
    if(b_ok) {
        ui->err_tip->setProperty("state", "normal");
    } else {
        ui->err_tip->setProperty("state", "err");
    }
    ui->err_tip->setText(str);

    repolish(ui->err_tip);
}

void LoginDialog::slot_forget_pwd()
{
    emit switchReset();
}

void LoginDialog::on_login_btn_clicked()
{
    if(!checkEmailValid()) {
        return;
    }

    if(!checkPassValid()) {
        return;
    }

    QJsonObject json_obj;

    json_obj["email"] = ui->email_edit->text();
    json_obj["passwd"] = md5Encrypt(ui->pass_edit->text());
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_login"),
                                        json_obj, ReqId::ID_LOGIN_USER, Modules::LOGINMOD);
}

void LoginDialog::slot_login_mod_finish(ReqId id, QString res, ErrorCodes err)
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

