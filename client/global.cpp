#include "global.h"
#include <QCryptographicHash>

QString gate_url_prefix = "";

std::function<void(QWidget*)> repolish = [](QWidget* w) {
    w->style()->unpolish(w);
    w->style()->polish(w);
};

QString md5Encrypt(const QString &input)
{
    QByteArray data = input.toUtf8();

    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Md5);

    return hash.toHex();
}
