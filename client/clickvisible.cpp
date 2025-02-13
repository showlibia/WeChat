#include "clickvisible.h"
#include <QMouseEvent>
#include <QStyleOptionFrame>
#include <QStyle>

ClickVisible::ClickVisible(QWidget *parent, QLineEdit* pass_edit, QLineEdit* confirm_edit)
    : QWidget(parent), _pass_edit(pass_edit), _confirm_edit(confirm_edit) {
    _pass_action = createPasswordAction(pass_edit);
    if (_confirm_edit) {
        _confirm_action = createPasswordAction(_confirm_edit);
        connect(_confirm_action, &QAction::triggered, this, [this]() {
            if (_confirm_edit && _confirm_action) {
                togglePasswordVisibility(_confirm_edit, _confirm_action);
            }
        });
    } else {
        _confirm_action = nullptr;
    }

    connect(_pass_action, &QAction::triggered, this, [this]() {
        togglePasswordVisibility(_pass_edit, _pass_action);
    });

    setupHoverEffects();
}

QAction *ClickVisible::createPasswordAction(QLineEdit *edit)
{
    if (!edit) {
        return nullptr;
    }
    QAction *action = new QAction(edit);
    action->setIcon(QIcon(hiddenIcons.normal));
    edit->addAction(action, QLineEdit::TrailingPosition);
    action->setProperty("isHidden", true);
    return action;
}

void ClickVisible::setupHoverEffects()
{
    _pass_edit->installEventFilter(this);
    if (_confirm_edit)
        _confirm_edit->installEventFilter(this);

    // 设置鼠标跟踪
    _pass_edit->setMouseTracking(true);
    if (_confirm_edit)
        _confirm_edit->setMouseTracking(true);
}

void ClickVisible::togglePasswordVisibility(QLineEdit *edit, QAction *action)
{
    if (!edit || !action) return;
    bool isHidden = (edit->echoMode() == QLineEdit::Password);

    // 切换密码可见性
    edit->setEchoMode(isHidden ? QLineEdit::Normal : QLineEdit::Password);
    action->setProperty("isHidden", !isHidden);

    // 更新图标状态
    const IconPaths &icons = isHidden ? visibleIcons : hiddenIcons;
    action->setIcon(QIcon(icons.normal));
}

bool ClickVisible::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == _pass_edit || (_confirm_edit && obj == _confirm_edit)) {
        QLineEdit *edit = qobject_cast<QLineEdit*>(obj);
        QAction *action = (edit == _pass_edit) ? _pass_action : _confirm_action;

        switch (event->type()) {
        case QEvent::MouseMove:
        case QEvent::HoverMove: {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            checkHoverState(edit, action, me->pos());
            break;
        }
        case QEvent::Leave:
            resetIconState(edit, action);
            edit->unsetCursor();
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void ClickVisible::checkHoverState(QLineEdit *edit, QAction *action, const QPoint &pos)
{
    if (!edit || !action) return;
    // 获取动作按钮的位置区域
    QStyleOptionFrame option;

    // 手动设置样式选项字段（比如矩形区域，状态等）
    option.rect = edit->rect();  // 控件的区域
    option.state = QStyle::State_Enabled | QStyle::State_Active;  // 设置控件的状态

    // 获取 QLineEdit 的样式
    QStyle *style = edit->style();

    // 获取 QLineEdit 内容区域的矩形（模拟 initStyleOption 的功能）
    QRect rect = style->subElementRect(QStyle::SE_LineEditContents, &option, edit);

    // 计算动作图标区域（右侧图标区域）
    // 计算右侧边界：使用 rect.x() + rect.width() 代替 rect.right()
    const int rightBoundary = rect.x() + rect.width();
    const int iconHeight = 25;
    const int iconWidth = 20;
    const int margin = 2;  // 添加边距以提高精确度
    QRect actionRect(
        rect.right() - iconWidth - margin,
        (rect.height() - iconHeight) / 2 + rect.top(),  // 垂直居中
        iconWidth,
        iconHeight
    );

    // 检查鼠标是否在图标区域内
    if (actionRect.contains(pos)) {
        edit->setCursor(Qt::PointingHandCursor);
        updateHoverIcon(action, true);
    } else {
        edit->setCursor(Qt::IBeamCursor);  // 将默认光标设为文本编辑光标
        updateHoverIcon(action, false);
    }
}

void ClickVisible::updateHoverIcon(QAction *action, bool hover)
{
    if (!action) return;
    bool isHidden = action->property("isHidden").toBool();
    const IconPaths &icons = isHidden ? hiddenIcons : visibleIcons;
    action->setIcon(QIcon(hover ? icons.hover : icons.normal));
}

void ClickVisible::resetIconState(QLineEdit *edit, QAction *action)
{
    if (!action) return;
    bool isHidden = action->property("isHidden").toBool();
    action->setIcon(QIcon(isHidden ? hiddenIcons.normal : visibleIcons.normal));
}
