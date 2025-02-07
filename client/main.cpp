#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include "global.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile qss(":/style/stylesheet.qss");
    if (qss.open(QFile::ReadOnly)) {
		qDebug("open qss file success");
		QString style = QLatin1String(qss.readAll());
		a.setStyleSheet(style);
		qss.close();
    }
    else {
		qDebug("open qss file failed");
    }

    // // 获取当前应用路径
    // QString app_path = QCoreApplication::applicationDirPath();
    // // 拼接配置文件名
    // QString fileName = "config.ini";
    // QString config_path = QDir::toNativeSeparators(app_path + QDir::separator() + fileName);

    QString app_path = ":/config.ini";
    QString config_path = QDir::toNativeSeparators(app_path);

    QSettings settings(config_path, QSettings::IniFormat);
    QString gate_host = settings.value("GateServer/host").toString();
    QString gate_port = settings.value("GateServer/port").toString();
    // if(gate_host.isEmpty() || gate_port.isEmpty()) {
    //     qDebug() << "gate_host or gate_port is Empty";
    // }
    gate_url_prefix = "http://" + gate_host + ":" + gate_port;

    MainWindow w;
    w.show();
    return a.exec();
}
