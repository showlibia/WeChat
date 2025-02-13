#ifndef TIMERBTN_H
#define TIMERBTN_H

#include <QPushButton>
#include <QTimer>

class TimerBtn : public QPushButton
{
    Q_OBJECT
public:
    TimerBtn(QWidget *parent = nullptr);
    ~TimerBtn();

    void mouseReleaseEvent(QMouseEvent *e) override;
    void startTimer();
private:
    QTimer* _timer;
    int _counter;
};

#endif // TIMERBTN_H
