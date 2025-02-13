#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	// 创建登录对话框，设置为主窗口的中央窗口
	_loginDialog = new LoginDialog(this);
    _loginDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
	setCentralWidget(_loginDialog);

    // 连接登录界面注册信号
	connect(_loginDialog, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    // 连接登录界面重置密码信号
    connect(_loginDialog, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SlotSwitchReg()
{
	// 切换到注册对话框
    _registerDialog = new RegisterDialog(this);

    _registerDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    // 连接注册界面返回登录界面的信号
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

    // 连接登录界面返回注册界面的信号
    connect(_loginDialog, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    // 连接登录界面重置密码信号
    connect(_loginDialog, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
}

void MainWindow::SlotSwitchReset()
{
    _resetDialog = new ResetDialog(this);
    _resetDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_resetDialog);

    _loginDialog->hide();
    _resetDialog->show();
    // 连接重置界面返回登录界面的信号
    connect(_resetDialog, &ResetDialog::sigSwitchLogin, this, &MainWindow::SlotSwitchLoginFromReset);
}

void MainWindow::SlotSwitchLoginFromReset()
{
    _loginDialog = new LoginDialog(this);
    _loginDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_loginDialog);

    _resetDialog->hide();
    _loginDialog->show();
    // 连接登录界面注册信号
    connect(_loginDialog, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    // 连接登录界面重置密码信号
    connect(_loginDialog, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
}
