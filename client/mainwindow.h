#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "logindialog.h"
#include "registerdialog.h"
#include "resetdialog.h"
#include "chatdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void SlotSwitchReg();
    void SlotSwitchLogin();
    void SlotSwitchReset();
    void SlotSwitchLoginFromReset();
    void SlotSwitchChat();
private:
    Ui::MainWindow *ui;
    LoginDialog* _loginDialog;
    RegisterDialog* _registerDialog;
    ResetDialog* _resetDialog;
    ChatDialog * _chatDialog;
};
#endif // MAINWINDOW_H
