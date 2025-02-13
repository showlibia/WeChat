#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	// ������¼�Ի�������Ϊ�����ڵ����봰��
	_loginDialog = new LoginDialog(this);
    _loginDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
	setCentralWidget(_loginDialog);

    // ���ӵ�¼����ע���ź�
	connect(_loginDialog, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    // ���ӵ�¼�������������ź�
    connect(_loginDialog, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SlotSwitchReg()
{
	// �л���ע��Ի���
    _registerDialog = new RegisterDialog(this);

    _registerDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    // ����ע����淵�ص�¼������ź�
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

    // ���ӵ�¼���淵��ע�������ź�
    connect(_loginDialog, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    // ���ӵ�¼�������������ź�
    connect(_loginDialog, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
}

void MainWindow::SlotSwitchReset()
{
    _resetDialog = new ResetDialog(this);
    _resetDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_resetDialog);

    _loginDialog->hide();
    _resetDialog->show();
    // �������ý��淵�ص�¼������ź�
    connect(_resetDialog, &ResetDialog::sigSwitchLogin, this, &MainWindow::SlotSwitchLoginFromReset);
}

void MainWindow::SlotSwitchLoginFromReset()
{
    _loginDialog = new LoginDialog(this);
    _loginDialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    setCentralWidget(_loginDialog);

    _resetDialog->hide();
    _loginDialog->show();
    // ���ӵ�¼����ע���ź�
    connect(_loginDialog, &LoginDialog::switchRegister, this, &MainWindow::SlotSwitchReg);
    // ���ӵ�¼�������������ź�
    connect(_loginDialog, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);
}
