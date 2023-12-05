#include "DateTimeManagerCPP.h"

#include <QTimeZone>

DateTimeManagerCPP::DateTimeManagerCPP(QObject *parent)
    : QObject{parent}
{}

QVariantList DateTimeManagerCPP::timezones() const
{
    QVariantList timezones;

    QDateTime now = QDateTime::currentDateTime();
    const QList<QByteArray>& timezoneIds = QTimeZone::availableTimeZoneIds();
    for (const QByteArray& tzId: timezoneIds) {
        QTimeZone timezone(tzId);


        int minutesOffset = timezone.offsetFromUtc(now) / 60;

        QString sign = "-";
        if (minutesOffset >= 0) {
            sign = "+";
        } else {
            minutesOffset *= -1;
        }

        QString hourStr = QString::number(int(minutesOffset / 60));
        if (hourStr.size() < 2) hourStr.prepend('0');

        QString minStr = QString::number(minutesOffset % 60);
        if (minStr.size() < 2) minStr.prepend('0');

        QVariantMap timezoneMap;
        timezoneMap["name"] = timezone.id();
        timezoneMap["city"] = timezone.id().sliced(timezone.id().indexOf('/') + 1);
        timezoneMap["offset"] = "UTC" + sign + hourStr + ":" + minStr;

        timezones.push_back(timezoneMap);
    }

    return timezones;
}
