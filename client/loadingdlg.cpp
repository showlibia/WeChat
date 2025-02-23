#include "loadingdlg.h"
#include "ui_loadingdlg.h"

#include <QMovie>

LoadingDlg::LoadingDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoadingDlg)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground); // 背景透明

    setFixedSize(parent->size());

    QMovie *movie = new QMovie(":/res/loading.gif");
    ui->loading_lb->setMovie(movie);
    ui->loading_lb->setAlignment(Qt::AlignCenter);
    movie->setScaledSize(QSize(30, 30));
    movie->start();
}

LoadingDlg::~LoadingDlg()
{
    delete ui;
}
