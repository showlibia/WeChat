#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	// 创建登录对话框，设置为主窗口的中央窗口
	_loginDialog = new LoginDialog(this);
	setCentralWidget(_loginDialog);

	// 切换到注册对话框, 由登录对话框发出的信号
	connect(_loginDialog, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);

	_registerDialog = new RegisterDialog(this);
    _registerDialog->hide();

    _loginDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    _registerDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SlotSwitchReg()
{
	// 切换到注册对话框
	setCentralWidget(_registerDialog);
	_loginDialog->hide();
	_registerDialog->show();
}
