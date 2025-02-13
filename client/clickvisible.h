#ifndef CLICKVISIBLE_H
#define CLICKVISIBLE_H

#include <QWidget>
#include <QLineEdit>
#include <QEvent>

class ClickVisible : public QWidget
{
    Q_OBJECT
public:
    ClickVisible(QWidget *parent = nullptr, QLineEdit* pass_edit = nullptr, QLineEdit* confirm_edit = nullptr);
private:
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

    QLineEdit *_pass_edit;
    QLineEdit *_confirm_edit;
    QAction *_pass_action;
    QAction *_confirm_action;
};

#endif // CLICKVISIBLE_H
