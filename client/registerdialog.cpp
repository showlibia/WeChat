#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "httpmgr.h"
#include "global.h"
#include <QMouseEvent>
#include <QStyleOptionFrame>

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);

    pass_action = createPasswordAction(ui->pass_edit);
    confirm_action = createPasswordAction(ui->confirm_edit);

    connect(pass_action, &QAction::triggered, this, [this]() {
        togglePasswordVisibility(ui->pass_edit, pass_action);
    });

    connect(confirm_action, &QAction::triggered, this, [this]() {
        togglePasswordVisibility(ui->confirm_edit, confirm_action);
    });

    setupHoverEffects();

    ui->err_tip->setProperty("state", "normal");
    repolish(ui->err_tip);

    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);
    initHttpHandlers();

    ui->err_tip->clear();
    connect(ui->user_edit, &QLineEdit::editingFinished, this,[this](){
        checkUserValid();
    });

    connect(ui->email_edit, &QLineEdit::editingFinished, this,[this](){
        checkEmailValid();
    });

    connect(ui->pass_edit, &QLineEdit::editingFinished, this, [this](){
        checkPassValid();
    });
    connect(ui->confirm_edit, &QLineEdit::editingFinished, this, [this](){
        checkConfirmValid();
    });
    connect(ui->verify_edit, &QLineEdit::editingFinished, this, [this](){
        checkVerifyValid();
    });
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_get_code_clicked()
{
    auto email = ui->email_edit->text();
    QJsonObject json_obj;
    json_obj["email"] = email;
    HttpMgr::GetInstance()->PostHttpReq(
        QUrl(gate_url_prefix + "/get_verifycode"),
        json_obj,
        ReqId::ID_GET_VERIFY_CODE,
        Modules::REGESTERMOD
        );
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

    _handlers.insert(ReqId::ID_REG_USER, [this](const QJsonObject& json){
        int err = json["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            showTip(tr("参数错误"), false);
            return;
        }
        auto email = json["email"].toString();
        showTip(tr("用户注册成功"), true);
        qDebug() << "email is " << email;
    });
}

bool RegisterDialog::checkUserValid()
{
    auto user = ui->user_edit->text();
    if(user.isEmpty()) {
        AddTipErr(tr("用户名不能为空"), TipErr::TIP_USER_ERR);
        return false;
    }

    DelTipErr(TipErr::TIP_USER_ERR);
    return true;
}

bool RegisterDialog::checkEmailValid()
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

bool RegisterDialog::checkPassValid()
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

bool RegisterDialog::checkConfirmValid()
{
    auto pass = ui->pass_edit->text();
    auto confirm = ui->confirm_edit->text();

    if(confirm != pass) {
        AddTipErr(tr("确认密码和密码不匹配"), TipErr::TIP_CONFIRM_ERR);
        return false;
    }

    DelTipErr(TipErr::TIP_CONFIRM_ERR);
    return true;
}

bool RegisterDialog::checkVerifyValid()
{
    auto verify_code = ui->verify_edit->text();
    if(verify_code.isEmpty()) {
        AddTipErr(tr("验证码不能为空"), TipErr::TIP_VARIFY_ERR);
        return false;
    }
    DelTipErr(TipErr::TIP_VARIFY_ERR);
    return true;
}

void RegisterDialog::AddTipErr(QString tips, TipErr te)
{
    _tip_errs[te] = tips;
    showTip(tips, false);
}

void RegisterDialog::DelTipErr(TipErr te)
{
    _tip_errs.remove(te);
    if(_tip_errs.empty()) {
        ui->err_tip->clear();
        return;
    }

    showTip(_tip_errs.first(), false);
}

QAction *RegisterDialog::createPasswordAction(QLineEdit *edit)
{
    QAction *action = new QAction(edit);
    action->setIcon(QIcon(hiddenIcons.normal));
    edit->addAction(action, QLineEdit::TrailingPosition);
    action->setProperty("isHidden", true);
    return action;
}

void RegisterDialog::setupHoverEffects()
{
    ui->pass_edit->installEventFilter(this);
    ui->confirm_edit->installEventFilter(this);

    // 设置鼠标跟踪
    ui->pass_edit->setMouseTracking(true);
    ui->confirm_edit->setMouseTracking(true);
}

void RegisterDialog::togglePasswordVisibility(QLineEdit *edit, QAction *action)
{
    bool isHidden = (edit->echoMode() == QLineEdit::Password);

    // 切换密码可见性
    edit->setEchoMode(isHidden ? QLineEdit::Normal : QLineEdit::Password);
    action->setProperty("isHidden", !isHidden);

    // 更新图标状态
    const IconPaths &icons = isHidden ? visibleIcons : hiddenIcons;
    action->setIcon(QIcon(icons.normal));
}

bool RegisterDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->pass_edit || obj == ui->confirm_edit) {
        QLineEdit *edit = qobject_cast<QLineEdit*>(obj);
        QAction *action = (edit == ui->pass_edit) ? pass_action : confirm_action;

        switch (event->type()) {
        case QEvent::MouseMove:
        case QEvent::HoverMove: {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            checkHoverState(edit, action, me->pos());
            break;
        }
        case QEvent::Leave:
            resetIconState(edit, action);
            edit->unsetCursor();
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void RegisterDialog::checkHoverState(QLineEdit *edit, QAction *action, const QPoint &pos)
{
    // 获取动作按钮的位置区域
    QStyleOptionFrame option;

    // 手动设置样式选项字段（比如矩形区域，状态等）
    option.rect = edit->rect();  // 控件的区域
    option.state = QStyle::State_Enabled | QStyle::State_Active;  // 设置控件的状态

    // 获取 QLineEdit 的样式
    QStyle *style = edit->style();

    // 获取 QLineEdit 内容区域的矩形（模拟 initStyleOption 的功能）
    QRect rect = style->subElementRect(QStyle::SE_LineEditContents, &option, edit);

    // 计算动作图标区域（右侧20x20区域）
    const int iconSize = 20;
    QRect actionRect(rect.right() - iconSize, rect.top(), iconSize, iconSize);

    if (actionRect.contains(pos)) {
        edit->setCursor(Qt::PointingHandCursor);
        updateHoverIcon(action, true);
    } else {
        edit->unsetCursor();
        updateHoverIcon(action, false);
    }
}

void RegisterDialog::updateHoverIcon(QAction *action, bool hover)
{
    bool isHidden = action->property("isHidden").toBool();
    const IconPaths &icons = isHidden ? hiddenIcons : visibleIcons;
    action->setIcon(QIcon(hover ? icons.hover : icons.normal));
}

void RegisterDialog::resetIconState(QLineEdit *edit, QAction *action)
{
    bool isHidden = action->property("isHidden").toBool();
    action->setIcon(QIcon(isHidden ? hiddenIcons.normal : visibleIcons.normal));
}

void RegisterDialog::on_confirm_button_clicked()
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
    json_obj["passwd"] = ui->pass_edit->text();
    json_obj["confirm"] = ui->confirm_edit->text();
    json_obj["verifycode"] = ui->verify_edit->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_register"),
                                        json_obj, ReqId::ID_REG_USER,Modules::REGESTERMOD);
}

