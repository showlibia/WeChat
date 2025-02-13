#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    connect(ui->register_button, &QPushButton::clicked, this, &LoginDialog::switchRegister);
    ui->forget_label->SetState("normal", "hover", "", "selected", "seleced_hover", "");
    ui->forget_label->setCursor(Qt::PointingHandCursor);

    connect(ui->forget_label, &ClickLabel::clicked, this, &LoginDialog::slot_forget_pwd);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::slot_forget_pwd()
{
    emit switchReset();
}
