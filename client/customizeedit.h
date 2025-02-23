#ifndef CUSTOMIZEEDIT_H
#define CUSTOMIZEEDIT_H

#include <QLineEdit>

class CustomizeEdit : public QLineEdit
{
public:
    CustomizeEdit(QWidget *parent = nullptr);
protected:
    void focusOutEvent(QFocusEvent *) override;
private:

signals:
    void sig_focus_out();
};

#endif // CUSTOMIZEEDIT_H
