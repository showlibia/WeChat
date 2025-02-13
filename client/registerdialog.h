#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include "global.h"
#include "clickvisible.h"
#include "timerbtn.h"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

private slots:
    void on_get_code_clicked();
    void on_confirm_button_clicked();

public slots:
    void slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
private:
    void initHttpHandlers();
    bool checkUserValid();
    bool checkEmailValid();
    bool checkPassValid();
    bool checkConfirmValid();
    bool checkVerifyValid();
    void AddTipErr(QString tips, TipErr te);
    void DelTipErr(TipErr te);

    Ui::RegisterDialog *ui;
    void showTip(QString str, bool b_ok);
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    QMap<TipErr, QString> _tip_errs;
    ClickVisible *_click_visible;
};

#endif // REGISTERDIALOG_H
