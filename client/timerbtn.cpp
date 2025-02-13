#include "timerbtn.h"
#include <QMouseEvent>

TimerBtn::TimerBtn(QWidget *parent):QPushButton(parent), _counter(10)
{
    _timer = new QTimer(this);

    connect(_timer, &QTimer::timeout, [this](){
        _counter--;
        if(_counter <= 0) {
            _timer->stop();
            _counter = 10;
            this->setText("获取");
            this->setEnabled(true);
            return;
        }
        this->setText(QString::number(_counter));
    });
}

TimerBtn::~TimerBtn()
{
    _timer->stop();
}

void TimerBtn::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        // 先发送点击信号，让外部处理邮箱验证
        emit clicked();
        
        // 不在这里直接开始计时，移除以下代码
        // this->setEnabled(false);
        // this->setText(QString::number(_counter));
        // _timer->start(1000);
    }

    // 调用基类的mouseReleaseEvent以确保正常的事件处理
    QPushButton::mouseReleaseEvent(e);
}

// 添加一个新的公共方法来启动计时器
void TimerBtn::startTimer()
{
    this->setEnabled(false);
    this->setText(QString::number(_counter));
    _timer->start(1000);
}
