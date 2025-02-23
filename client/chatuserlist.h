#ifndef CHATUSERLIST_H
#define CHATUSERLIST_H

#include<QListWidget>

class ChatUserList : public QListWidget
{
    Q_OBJECT
public:
    ChatUserList(QWidget* parent = nullptr);
protected:
    bool eventFilter(QObject *object, QEvent *event) override;
private slots:
    void checkScrollEnd();
signals:
    void sig_loading_chat_user();
};

#endif // CHATUSERLIST_H
