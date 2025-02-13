#ifndef RESETDIALOG_H
#define RESETDIALOG_H

#include <QDialog>
#include "global.h"
#include "clickvisible.h"

namespace Ui {
class ResetDialog;
}

class ResetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ResetDialog(QWidget *parent = nullptr);
    ~ResetDialog();
private:
    void initHttpHandlers();
    bool checkUserValid();
    bool checkEmailValid();
    bool checkPassValid();
    bool checkVerifyValid();
    void AddTipErr(QString tips, TipErr te);
    void DelTipErr(TipErr te);

    Ui::ResetDialog *ui;
    void showTip(QString str, bool b_ok);
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    QMap<TipErr, QString> _tip_errs;
    ClickVisible* _click_visible;

signals:
    void sigSwitchLogin();
private slots:
    void on_get_code_clicked();
    void on_confirm_btn_clicked();
    void on_return_btn_clicked();
    void slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err);
};

#endif // RESETDIALOG_H
