#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	// ������¼�Ի�������Ϊ�����ڵ����봰��
	_loginDialog = new LoginDialog(this);
	setCentralWidget(_loginDialog);

	// �л���ע��Ի���, �ɵ�¼�Ի��򷢳����ź�
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
	// �л���ע��Ի���
	setCentralWidget(_registerDialog);
	_loginDialog->hide();
	_registerDialog->show();
}
