#include "chatuserlist.h"
#include <QEvent>
#include <QWheelEvent>
#include <QScrollBar>

ChatUserList::ChatUserList(QWidget *parent) : QListWidget(parent)
{
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->viewport()->installEventFilter(this);

    connect(verticalScrollBar(), &QScrollBar::valueChanged,
            this, &ChatUserList::checkScrollEnd);
}

bool ChatUserList::eventFilter(QObject *object, QEvent *event)
{
    // 检查事件是否是鼠标悬浮进入或离开
    if(object == this->viewport()) {
        if(event->type() == QEvent::Enter) {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        } else if (event->type() == QEvent::Leave) {
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }


    return QListWidget::eventFilter(object, event);
}

void ChatUserList::checkScrollEnd()
{
    QScrollBar* scrollBar = verticalScrollBar();
    if (scrollBar->value() == scrollBar->maximum()) {
        qDebug() << "Scrolled to bottom, loading more...";
        emit sig_loading_chat_user();
    }
}

