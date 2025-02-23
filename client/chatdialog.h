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
private:
    Ui::ChatDialog *ui;
    ChatUIMode _state;
    ChatUIMode _mode;
};

#endif // CHATDIALOG_H
