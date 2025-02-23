#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include "global.h"

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
    void ShowSearch(bool search);
    void addChatUserList();
private slots:
    void slot_loading_chat_user();
private:
    Ui::ChatDialog *ui;
    bool _b_loading;
    ChatUIMode _state;
    ChatUIMode _mode;
};

#endif // CHATDIALOG_H
