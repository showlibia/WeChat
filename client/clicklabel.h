#ifndef CLICKLABEL_H
#define CLICKLABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QEvent>
#include <QWidget>
#include "global.h"

class ClickLabel : public QLabel
{
    Q_OBJECT
public:
    ClickLabel(QWidget *parent=nullptr);
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent* event) override;

    void SetState(const QString& normal = "",
                  const QString& hover = "",
                  const QString& press = "",
                  const QString& select = "",
                  const QString& select_hover = "",
                  const QString& select_press = "");

    ClickLbState getState();
    bool SetCurState(ClickLbState state);
    void ResetNormalState();
signals:
    void clicked(QString, ClickLbState);
private:
    QString _normal;
    QString _normal_hover;
    QString _normal_press;

    QString _selected;
    QString _selected_hover;
    QString _selected_press;

    ClickLbState _state;
};

#endif // CLICKLABEL_H
