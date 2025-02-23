#include "chatuserwid.h"
#include "ui_chatuserwid.h"

ChatUserWid::ChatUserWid(QWidget *parent)
    : ListItemBase(parent)
    , ui(new Ui::ChatUserWid)
{
    ui->setupUi(this);
    SetItemType(ListItemType::CHAT_USER_ITEM);
}

ChatUserWid::~ChatUserWid()
{
    delete ui;
}

void ChatUserWid::SetInfo(QString name, QString head, QString msg)
{
    _name = name;
    _head = head;
    _msg = msg;
    // 加载图片
    QPixmap pixmap(_head);
    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    // 设置用户名显示省略号
    QFontMetrics nameFontMetrics(ui->user_name_lb->font());
    int nameWidth = ui->user_name_lb->width(); // 获取 QLabel 的宽度
    QString elidedName = nameFontMetrics.elidedText(_name, Qt::ElideRight, nameWidth);

    QFontMetrics msgFontMetrics(ui->user_chat_lb->font());
    int msgWidth = ui->user_chat_lb->width(); // 获取 QLabel 的宽度
    QString elidedMsg = msgFontMetrics.elidedText(_msg, Qt::ElideRight, msgWidth);

    ui->user_name_lb->setText(elidedName);
    ui->user_chat_lb->setText(elidedMsg);
}
