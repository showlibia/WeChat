#include "clicklabel.h"

ClickLabel::ClickLabel(QWidget *parent): QLabel(parent) {
    setCursor(Qt::PointingHandCursor);
}

// 处理鼠标点击事件
void ClickLabel::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton) {
        if(_state == ClickLbState::Normal) {
            _state = ClickLbState::Selected;
            setProperty("state", _selected_press);
            repolish(this);
            update();
        } else {
            _state = ClickLbState::Normal;
            setProperty("state", _normal_press);
            repolish(this);
            update();
        }
        return;
    }
    QLabel::mousePressEvent(ev);
}

void ClickLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton) {
        if(_state == ClickLbState::Normal) {
            _state = ClickLbState::Selected;
            setProperty("state", _normal_hover);
            repolish(this);
            update();
        } else {
            _state = ClickLbState::Normal;
            setProperty("state", _selected_hover);
            repolish(this);
            update();
        }
        emit clicked(this->text(), _state);
        return;
    }

    QLabel::mouseReleaseEvent(ev);
}

// 鼠标进入悬停
void ClickLabel::enterEvent(QEnterEvent *event)
{
    if(_state == ClickLbState::Normal) {
        setProperty("state", _normal_hover);
        repolish(this);
        update();
    } else {
        setProperty("state", _selected_hover);
        repolish(this);
        update();
    }
    QLabel::enterEvent(event);
}

void ClickLabel::leaveEvent(QEvent *event)
{
    if(_state == ClickLbState::Normal) {
        setProperty("state", _normal);
        repolish(this);
        update();
    } else {
        setProperty("state", _selected);
        repolish(this);
        update();
    }
    QLabel::leaveEvent(event);
}

void ClickLabel::SetState(const QString &normal, const QString &hover, const QString &press, const QString &select, const QString &select_hover, const QString &select_press)
{
    _normal = normal;
    _normal_hover = hover;
    _normal_press = press;

    _selected = select;
    _selected_hover = select_hover;
    _selected_press = select_press;

    setProperty("state",normal);
    repolish(this);
}

ClickLbState ClickLabel::getState()
{
    return _state;
}

bool ClickLabel::SetCurState(ClickLbState state)
{
    _state = state;
    if (_state == ClickLbState::Normal) {
        setProperty("state", _normal);
        repolish(this);
    }
    else if (_state == ClickLbState::Selected) {
        setProperty("state", _selected);
        repolish(this);
    }

    return true;
}

void ClickLabel::ResetNormalState()
{
    _state = ClickLbState::Normal;
    setProperty("state", _normal);
    repolish(this);
}

