#include "customizeedit.h"

CustomizeEdit::CustomizeEdit(QWidget *parent): QLineEdit(parent)
{

}

void CustomizeEdit::focusOutEvent(QFocusEvent *event)
{
    QLineEdit::focusOutEvent(event);
    // emit sig_focus_out();
}
