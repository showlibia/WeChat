#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include "global.h"

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

    // 为pass_edit和confirm_edit添加带图标的action
    QAction* createPasswordAction(QLineEdit *edit);
    // 安装事件过滤器, 追踪鼠标事件
    void setupHoverEffects();
    // 随图标切换密码可见性
    void togglePasswordVisibility(QLineEdit *edit, QAction *action);
    // 处理密码框的鼠标事件
    bool eventFilter(QObject *obj, QEvent *event) override;
    // 检查是否为悬停状态
    void checkHoverState(QLineEdit *edit, QAction *action, const QPoint &pos);
    // 更新悬停状态的icon
    void updateHoverIcon(QAction *action, bool hover);
    // 当更新为非悬停状态的icon
    void resetIconState(QLineEdit *edit, QAction *action);

    struct IconPaths {
        QString normal;
        QString hover;
    };

    IconPaths visibleIcons{":/res/visible.png", ":/res/visible_hover.png"};
    IconPaths hiddenIcons{":/res/unvisible.png", ":/res/unvisible_hover.png"};

    Ui::RegisterDialog *ui;
    QAction *pass_action;
    QAction *confirm_action;
    void showTip(QString str, bool b_ok);
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    QMap<TipErr, QString> _tip_errs;
};

#endif // REGISTERDIALOG_H
