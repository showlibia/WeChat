#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include "global.h"
#include "statewidget.h"

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
protected:
    bool eventFilter(QObject *watched, QEvent *event) override ;
    void handleGlobalMousePress(QMouseEvent *event) ;
    void CloseFindDlg();
public slots:
    void slot_loading_chat_user();
    void slot_side_chat();
    void slot_side_contact();
    void slot_text_changed(const QString &str);
private:
    void AddLBGroup(StateWidget* lb);
    void ClearLabelState(StateWidget* lb);
    Ui::ChatDialog *ui;
    bool _b_loading;
    ChatUIMode _state;
    ChatUIMode _mode;
    QList<StateWidget*> _lb_list;
};

#endif // CHATDIALOG_H
