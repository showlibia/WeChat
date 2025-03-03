#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "tcpmgr.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //
    _loginDialog = new LoginDialog(this);
    _loginDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_loginDialog);

    //
    connect(_loginDialog, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    //
    connect(_loginDialog, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
    //
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_swich_chatdlg, this, &MainWindow::SlotSwitchChat);
    // emit TcpMgr::GetInstance()->sig_swich_chatdlg();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SlotSwitchReg()
{
    // 
    _registerDialog = new RegisterDialog(this);

    _registerDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    // Y
    connect(_registerDialog, &RegisterDialog::sigSwitchLogin, this, &MainWindow::SlotSwitchLogin);
    setCentralWidget(_registerDialog);
    _loginDialog->hide();
    _registerDialog->show();
}

void MainWindow::SlotSwitchLogin()
{
    _loginDialog = new LoginDialog(this);
    _loginDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_loginDialog);

    // Y
    connect(_loginDialog, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    //
    connect(_loginDialog, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
}

void MainWindow::SlotSwitchReset()
{
    _resetDialog = new ResetDialog(this);
    _resetDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_resetDialog);

    _loginDialog->hide();
    _resetDialog->show();
    // Y
    connect(_resetDialog, &ResetDialog::sigSwitchLogin, this, &MainWindow::SlotSwitchLoginFromReset);
}

void MainWindow::SlotSwitchLoginFromReset()
{
    _loginDialog = new LoginDialog(this);
    _loginDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_loginDialog);

    _resetDialog->hide();
    _loginDialog->show();
    //
    connect(_loginDialog, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    //
    connect(_loginDialog, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
}

void MainWindow::SlotSwitchChat()
{
    _chatDialog = new ChatDialog(this);
    _chatDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_chatDialog);

    _loginDialog->hide();
    _chatDialog->show();

    this->setMinimumSize(QSize(900,750));
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}
